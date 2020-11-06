use std::io::{Read, Write};
use std::net::SocketAddr;
use std::net::TcpListener;
use std::net::{Shutdown, TcpStream};
use std::sync::mpsc::{channel, Sender};
use std::thread;

#[derive(Debug)]
pub enum Message {
    Hoge,
}

#[derive(Debug)]
pub struct Bridge<'a> {
    address: &'a str,
    sender: Sender<Message>,
}

impl Bridge<'_> {
    pub fn new(ip: &str, sender: Sender<Message>) -> Bridge {
        Bridge {
            address: ip,
            sender: sender,
        }
    }

    pub fn handle_client(&mut self, mut stream: TcpStream) {
        let mut data = [0 as u8; 50];
        while match stream.read(&mut data) {
            Ok(size) => {
                // stream.write(&data[0..size]).unwrap();
                let topic_len = data[0] as usize;
                let topic_str = std::str::from_utf8(&data[1..topic_len + 1]).unwrap();
                let v: Vec<&str> = topic_str.split('/').collect();
                let payload_len = data[1 + topic_len] as usize;
                let count = data[1 + topic_len + 1] as usize;
                println!("{:?}", &data[0..size]);
                println!("{:?}", topic_str);
                println!("{:?}", v);
                println!("{:?}", count);
                self.sender.send(Message::Hoge).unwrap();
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
}

pub struct BridgeManager {}

impl BridgeManager {
    pub fn spawn() {
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
                            let mut bridge = Bridge::new(&ip, sender);
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
