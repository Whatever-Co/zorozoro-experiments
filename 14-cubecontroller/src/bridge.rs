use std::io::{Read, Write};
use std::net::{Shutdown, SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc::{channel, Sender};
use std::thread;

#[derive(Debug)]
pub enum Message {
    NewCubeFound(String),
    Available(usize),
    Unknown,
}

#[derive(Debug)]
pub struct Bridge {
    address: String,
    sender: Sender<Message>,
}

impl Bridge {
    pub fn new(ip: String, sender: Sender<Message>) -> Bridge {
        Bridge {
            address: ip,
            sender: sender,
        }
    }

    pub fn handle_client(&mut self, mut stream: TcpStream) {
        let mut data = [0 as u8; 50];
        while match stream.read(&mut data) {
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
                    stream.peer_addr().unwrap()
                );
                stream.shutdown(Shutdown::Both).unwrap();
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
                let address = std::str::from_utf8(payload).unwrap().to_string();
                Message::NewCubeFound(address)
            }
            "available" => {
                let count = payload[0] as usize;
                Message::Available(count)
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

        let (sender, receiver) = channel();

        thread::spawn(move || {
            for stearm in listener.incoming() {
                match stearm {
                    Ok(stream) => {
                        println!("New connection: {}", stream.peer_addr().unwrap());
                        let ip = match stream.peer_addr().unwrap() {
                            SocketAddr::V4(addr) => addr.ip().to_string(),
                            SocketAddr::V6(addr) => addr.ip().to_string(),
                        };
                        println!("ip: {:?}", ip);
                        let sender = sender.clone();
                        thread::spawn(move || {
                            let mut bridge = Bridge::new(ip, sender);
                            println!("{:?}", bridge);
                            bridge.handle_client(stream);
                        });
                    }
                    Err(e) => {
                        println!("Error: {}", e);
                    }
                }
            }
        });

        loop {
            println!("rcv: {:?}", receiver.recv().unwrap());
        }
    }
}
