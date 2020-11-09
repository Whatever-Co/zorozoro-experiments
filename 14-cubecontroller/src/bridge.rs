use byteorder::{LittleEndian, ReadBytesExt};
use std::io::{self, Read, Write};
use std::net::{Shutdown, SocketAddr, TcpStream};
use std::sync::mpsc::{Receiver, Sender};
use std::{thread, time};

#[derive(Debug)]
pub enum BridgeMode {
    Bridge,
    Scanner,
    Unknown,
}

#[derive(Debug)]
pub enum IDInfo {
    PositionID(u16, u16, u16),
    StandardID(u32, u16),
    PositionIDMissed,
    StandardIDMissed,
}

#[derive(Debug)]
pub enum Message {
    NewCubeFound(String),
    Available(String, usize),
    Connected(String, String),
    Disconnected(String, String),
    IDInfo(IDInfo),
    BatteryInfo(u8),
    SetLamp(String, u8, u8, u8),
    Unknown,
}

#[derive(Debug)]
pub struct Bridge {
    address: String,
    mode: BridgeMode,
    sender: Sender<Message>,
    receiver: Receiver<Message>,
    stream: TcpStream,
}

impl Bridge {
    pub fn new(sender: Sender<Message>, receiver: Receiver<Message>, stream: TcpStream) -> Bridge {
        let address = match stream.peer_addr().unwrap() {
            SocketAddr::V4(addr) => addr.ip().to_string(),
            SocketAddr::V6(addr) => addr.ip().to_string(),
        };
        Bridge {
            address,
            mode: BridgeMode::Unknown,
            sender,
            receiver,
            stream,
        }
    }

    pub fn start(&mut self) {
        let mut data = [0 as u8; 1024];
        self.stream.set_nonblocking(true).unwrap();
        while match self.stream.read(&mut data) {
            Ok(_size) => {
                self.process_message(&data);
                true
            }
            Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                if let Some(message) = self.receiver.try_recv().ok() {
                    self.send_message(message);
                }
                thread::sleep(time::Duration::from_millis(10));
                true
            }
            Err(_) => {
                println!("An error occurred, terminating connection with {}", self.stream.peer_addr().unwrap());
                self.stream.shutdown(Shutdown::Both).unwrap();
                false
            }
        } {}
    }

    fn process_message(&mut self, data: &[u8]) {
        let (address, command, payload) = {
            let topic_len = data[0] as usize;
            let topic = std::str::from_utf8(&data[1..topic_len + 1]).unwrap();
            let (address, command) = match topic.find('/') {
                Some(_) => {
                    let v: Vec<&str> = topic.split('/').collect();
                    (v[0], v[1])
                }
                _ => ("", topic),
            };
            let payload_len = data[1 + topic_len] as usize;
            let p = 1 + topic_len + 1;
            (address, command, &data[p..p + payload_len])
        };
        println!("address={:?}, command={:?}, payload={:?}", address, command, payload);
        let message = match command {
            "newcube" => {
                self.mode = BridgeMode::Scanner;
                let address = std::str::from_utf8(payload).unwrap().to_string();
                Message::NewCubeFound(address)
            }

            "available" => {
                self.mode = BridgeMode::Bridge;
                let slots = payload[0] as usize;
                Message::Available(self.address.clone(), slots)
            }

            "connected" => Message::Connected(self.address.clone(), address.to_string()),

            "disconnected" => Message::Disconnected(self.address.clone(), address.to_string()),

            "position" => match payload[0] {
                1 => {
                    let mut p = &payload[1..];
                    let x = p.read_u16::<LittleEndian>().unwrap();
                    let y = p.read_u16::<LittleEndian>().unwrap();
                    let a = p.read_u16::<LittleEndian>().unwrap();
                    let id = IDInfo::PositionID(x, y, a);
                    Message::IDInfo(id)
                }
                2 => {
                    let mut p = &payload[1..];
                    let value = p.read_u32::<LittleEndian>().unwrap();
                    let a = p.read_u16::<LittleEndian>().unwrap();
                    let id = IDInfo::StandardID(value, a);
                    Message::IDInfo(id)
                }
                3 => Message::IDInfo(IDInfo::PositionIDMissed),
                4 => Message::IDInfo(IDInfo::StandardIDMissed),
                _ => Message::Unknown,
            },

            "battery" => Message::BatteryInfo(payload[0]),

            &_ => Message::Unknown,
        };
        println!("message={:?}", message);
        self.sender.send(message).unwrap();
    }

    fn send_message(&mut self, message: Message) {
        match message {
            Message::NewCubeFound(cube_address) => {
                let mut buffer = Vec::<u8>::with_capacity(64);
                let topic = "/newcube";
                buffer.push(topic.len() as u8);
                buffer.extend(topic.as_bytes());
                buffer.push(cube_address.len() as u8);
                buffer.extend(cube_address.as_bytes());
                println!("len={:?}, buffer={:?}", buffer.len(), buffer);
                self.stream.write(&buffer).unwrap();
            }

            Message::SetLamp(cube_address, r, g, b) => {
                let mut buffer = Vec::<u8>::with_capacity(64);
                let topic = cube_address + "/lamp";
                buffer.push(topic.len() as u8);
                buffer.extend(topic.as_bytes());
                let payload = vec![0x03, 0x00, 0x01, 0x01, r, g, b];
                buffer.push(payload.len() as u8);
                buffer.extend(payload);
                println!("len={:?}, buffer={:?}", buffer.len(), buffer);
                self.stream.write(&buffer).unwrap();
            }

            _ => {}
        }
    }
}
