use std::io::{Read, Write};
use std::net::{Shutdown, SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc::{channel, Sender};
use std::thread;

#[derive(Debug)]
pub enum BridgeMode {
    Bridge,
    Scanner,
    Unknown,
}

#[derive(Debug)]
pub enum Message {
    NewCubeFound(String),
    Available(usize),
    Unknown,
}

#[derive(Debug)]
pub struct Bridge {
    address: String,
    mode: BridgeMode,
    available_slots: usize,
    sender: Sender<Message>,
    stream: TcpStream,
}

impl Bridge {
    pub fn new(sender: Sender<Message>, stream: TcpStream) -> Bridge {
        let ip = match stream.peer_addr().unwrap() {
            SocketAddr::V4(addr) => addr.ip().to_string(),
            SocketAddr::V6(addr) => addr.ip().to_string(),
        };
        Bridge {
            address: ip,
            mode: BridgeMode::Unknown,
            available_slots: 0,
            sender: sender,
            stream: stream,
        }
    }

    pub fn start(&mut self) {
        let mut data = [0 as u8; 50];
        while match self.stream.read(&mut data) {
            Ok(_size) => {
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
                self.process_message(address, command, &data[p..p + payload_len]);
                true
            }
            Err(_) => {
                println!(
                    "An error occurred, terminating connection with {}",
                    self.stream.peer_addr().unwrap()
                );
                self.stream.shutdown(Shutdown::Both).unwrap();
                false
            }
        } {}
    }

    pub fn process_message(&mut self, address: &str, command: &str, payload: &[u8]) {
        println!(
            "address={:?}, command={:?}, payload={:?}",
            address, command, payload
        );
        let message = match command {
            "newcube" => {
                self.mode = BridgeMode::Scanner;
                let address = std::str::from_utf8(payload).unwrap().to_string();
                Message::NewCubeFound(address)
            }
            "available" => {
                self.mode = BridgeMode::Bridge;
                self.available_slots = payload[0] as usize;
                Message::Available(self.available_slots)
            }
            &_ => Message::Unknown,
        };
        println!("message={:?}", message);
        self.sender.send(message).unwrap();
    }

    pub fn available_slots(&self) -> usize {
        self.available_slots
    }
}

pub struct BridgeManager {}

impl BridgeManager {
    pub fn start() {
        let listener = TcpListener::bind("0.0.0.0:11111").unwrap();
        println!("Server listening on port 11111");

        let (sender, receiver) = channel();

        thread::spawn(move || {
            for stearm in listener.incoming() {
                match stearm {
                    Ok(stream) => {
                        let sender = sender.clone();
                        thread::spawn(move || {
                            let mut bridge = Bridge::new(sender, stream);
                            println!("{:?}", bridge);
                            bridge.start();
                        });
                    }
                    Err(e) => {
                        println!("Error: {}", e);
                    }
                }
            }
        });

        loop {
            match receiver.recv().unwrap() {
                Message::NewCubeFound(address) => {}
                _ => {}
            };
        }
    }
}
