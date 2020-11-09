use crate::bridge::{Bridge, Message};
use crate::cube::CubeManager;
use std::collections::HashMap;
use std::net::{SocketAddr, TcpListener};
use std::sync::mpsc::channel;
use std::sync::{Arc, Mutex};
use std::thread;

#[derive(Debug)]
pub struct BridgeManager {
    cube_manager: Arc<Mutex<CubeManager>>,
}

impl BridgeManager {
    pub fn new(cube_manager: Arc<Mutex<CubeManager>>) -> BridgeManager {
        BridgeManager { cube_manager }
    }

    pub fn start(&mut self) {
        let listener = TcpListener::bind("0.0.0.0:11111").unwrap();
        println!("Server listening on port 11111");

        let mut available_bridges = HashMap::new();
        let senders_to_bridge = Arc::new(Mutex::new(HashMap::new()));

        let (sender1, receiver1) = channel();
        {
            let senders_to_bridge = senders_to_bridge.clone();
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
                            senders_to_bridge.lock().unwrap().insert(Arc::new(address), sender2);
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
        }

        loop {
            match receiver1.recv().unwrap() {
                Message::NewCubeFound(cube_address) => {
                    if let Some((bridge_address, _)) = available_bridges.iter().next() {
                        let address = Arc::clone(bridge_address);
                        available_bridges.remove(&address);
                        if let Some(sender) = senders_to_bridge.lock().unwrap().get(&address) {
                            println!("sender={:?}", sender);
                            sender.send(Message::NewCubeFound(cube_address)).unwrap();
                        };
                    }
                }

                Message::Available(address, slots) => {
                    available_bridges.insert(Arc::new(address), slots);
                }

                Message::Connected(bridge_address, cube_address) => {
                    self.cube_manager.lock().unwrap().add_new(bridge_address, cube_address);
                }
                _ => {}
            };
        }
    }
}
