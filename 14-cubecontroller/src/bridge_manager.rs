use crate::bridge::{Bridge, Message};
use std::collections::HashMap;
use std::net::{SocketAddr, TcpListener};
use std::sync::mpsc::{channel, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::thread;

#[derive(Debug)]
pub struct BridgeManager {
    to_cubes: Sender<Message>,
    from_cubes: Receiver<Message>,
}

impl BridgeManager {
    pub fn new(to_cubes: Sender<Message>, from_cubes: Receiver<Message>) -> BridgeManager {
        BridgeManager { to_cubes, from_cubes }
    }

    pub fn start(&mut self) {
        let listener = TcpListener::bind("0.0.0.0:11111").unwrap();
        println!("Server listening on port 11111");

        let senders_to_bridge = Arc::new(Mutex::new(HashMap::new()));

        let (to_manager, from_bridge) = channel();
        {
            let senders_to_bridge = senders_to_bridge.clone();
            thread::spawn(move || {
                for stearm in listener.incoming() {
                    match stearm {
                        Ok(stream) => {
                            let to_manager = to_manager.clone();
                            let (to_bridge, from_manager) = channel();
                            let address = match stream.peer_addr().unwrap() {
                                SocketAddr::V4(addr) => addr.ip().to_string(),
                                SocketAddr::V6(addr) => addr.ip().to_string(),
                            };
                            senders_to_bridge.lock().unwrap().insert(address, to_bridge);
                            thread::spawn(move || {
                                let mut bridge = Bridge::new(to_manager, from_manager, stream);
                                println!("{:?}", bridge);
                                bridge.start();
                            });
                        }
                        Err(e) => {
                            eprintln!("Error: {}", e);
                        }
                    }
                }
            });
        }

        let mut bridges = Vec::<(String, usize)>::with_capacity(32);
        loop {
            match from_bridge.recv().unwrap() {
                Message::NewCubeFound(cube_address) => {
                    if let Some((bridge_address, _slots)) = bridges.pop() {
                        if let Some(sender) = senders_to_bridge.lock().unwrap().get(&bridge_address) {
                            println!("sender={:?}", sender);
                            sender.send(Message::NewCubeFound(cube_address)).unwrap();
                        };
                    }
                }

                Message::Available(address, slots) => {
                    bridges.push((address, slots));
                    bridges.sort_by(|a, b| a.1.cmp(&b.1));
                }

                m @ Message::Connected(_, _) => self.to_cubes.send(m).unwrap(),

                _ => (),
            };
        }
    }
}
