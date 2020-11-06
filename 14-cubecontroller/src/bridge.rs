use std::collections::HashMap;
use std::io::{self, Read};
use std::net::{Shutdown, SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc::{channel, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::{thread, time};

#[derive(Debug)]
pub enum BridgeMode {
    Bridge,
    Scanner,
    Unknown,
}

#[derive(Debug)]
pub enum Message {
    NewCubeFound(String),
    Available(String, usize),
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
        let mut data = [0 as u8; 50];
        self.stream.set_nonblocking(true).expect("hohohoh");
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
            Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                thread::sleep(time::Duration::from_millis(3));
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
                let slots = payload[0] as usize;
                Message::Available(self.address.clone(), slots)
            }
            &_ => Message::Unknown,
        };
        println!("message={:?}", message);
        self.sender.send(message).unwrap();
    }
}

pub struct BridgeManager {}

impl BridgeManager {
    pub fn start() {
        let listener = TcpListener::bind("0.0.0.0:11111").unwrap();
        println!("Server listening on port 11111");

        let (sender1, receiver1) = channel();
        let mut available_bridges = HashMap::new();
        let senders_to_bridge = Arc::new(Mutex::new(HashMap::new()));

        let hoge_cuo = Arc::clone(&senders_to_bridge);

        thread::spawn(move || {
            for stearm in listener.incoming() {
                match stearm {
                    Ok(stream) => {
                        let sender1 = sender1.clone();
                        let (sender2, receiver2) = channel();
                        let address = match stream.peer_addr().unwrap() {
                            SocketAddr::V4(addr) => addr.ip().to_string(),
                            SocketAddr::V6(addr) => addr.ip().to_string(),
                        };
                        hoge_cuo.lock().unwrap().insert(Arc::new(address), sender2);
                        thread::spawn(move || {
                            let mut bridge = Bridge::new(sender1, receiver2, stream);
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
            match receiver1.recv().unwrap() {
                Message::NewCubeFound(_cube_address) => {
                    if let Some((_bridge_address, _)) = available_bridges.iter().next() {
                        let address = Arc::clone(_bridge_address);
                        available_bridges.remove(&address);
                        if let Some(a) = senders_to_bridge.lock().unwrap().get(&address) {};
                    }
                }
                Message::Available(address, slots) => {
                    available_bridges.insert(Arc::new(address), slots);
                }
                _ => {}
            };
        }
    }
}
