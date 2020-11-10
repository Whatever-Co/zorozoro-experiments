use crate::bridge::{Bridge, Message};
use crossbeam_channel::{select, unbounded, Receiver, Sender};
use std::collections::HashMap;
use std::net::{SocketAddr, TcpListener};
use std::sync::{Arc, Mutex};
use std::thread;

#[derive(Debug)]
pub struct BridgeManager {
    to_cubes: Sender<Message>,
    from_cubes: Receiver<Message>,
    senders_to_bridge: Arc<Mutex<HashMap<String, Sender<Message>>>>,
    bridges: Vec<(String, usize)>,
}

impl BridgeManager {
    pub fn new(to_cubes: Sender<Message>, from_cubes: Receiver<Message>) -> BridgeManager {
        BridgeManager {
            to_cubes,
            from_cubes,
            senders_to_bridge: Arc::new(Mutex::new(HashMap::new())),
            bridges: Vec::with_capacity(32),
        }
    }

    pub fn start(&mut self) {
        let listener = TcpListener::bind("0.0.0.0:11111").unwrap();
        println!("Server listening on port 11111");

        let (to_manager, from_bridge) = unbounded();
        {
            let senders_to_bridge = self.senders_to_bridge.clone();
            thread::spawn(move || {
                for stearm in listener.incoming() {
                    match stearm {
                        Ok(stream) => {
                            let to_manager = to_manager.clone();
                            let (to_bridge, from_manager) = unbounded();
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

        loop {
            select! {
                recv(from_bridge) -> message => self.process_message(&message.unwrap()),
                recv(self.from_cubes) -> message => self.process_message(&message.unwrap()),
            }
        }
    }

    fn process_message(&mut self, message: &Message) {
        match message {
            Message::NewCubeFound(cube_address) => {
                if let Some((bridge_address, _slots)) = self.bridges.pop() {
                    if let Some(sender) = self.senders_to_bridge.lock().unwrap().get(&bridge_address) {
                        println!("sender={:?}", sender);
                        sender.send(Message::NewCubeFound(cube_address.clone())).unwrap();
                    };
                }
            }

            Message::Available(address, slots) => {
                self.bridges.push((address.clone(), *slots));
                self.bridges.sort_by(|a, b| a.1.cmp(&b.1));
            }

            m @ Message::Connected(_, _) | m @ Message::Disconnected(_, _) | m @ Message::IDInfo(_, _) | m @ Message::BatteryInfo(_, _) => {
                self.to_cubes.send(m.clone()).unwrap()
            }

            Message::SetLamp(cube_address, bridge_address, r, g, b) => {
                if let Some(sender) = self.senders_to_bridge.lock().unwrap().get(bridge_address) {
                    println!("sender={:?}", sender);
                    sender
                        .send(Message::SetLamp(cube_address.clone(), bridge_address.clone(), *r, *g, *b))
                        .unwrap();
                }
            }

            _ => (),
        }
    }
}
