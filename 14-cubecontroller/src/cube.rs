use crate::bridge::Message;
use crossbeam_channel::{Receiver, Sender};
use std::collections::HashMap;

#[derive(Debug)]
pub struct Cube {
    address: String,
    bridge: String,
    to_bridge: Sender<Message>,
}

impl Cube {
    pub fn set_lamp(&self, red: u8, green: u8, blue: u8) {
        println!("set_lamp");
        self.to_bridge
            .send(Message::SetLamp(self.address.clone(), self.bridge.clone(), red, green, blue))
            .unwrap();
    }
}

#[derive(Debug)]
pub struct CubeManager {
    cubes: HashMap<String, Cube>,
    to_bridge: Sender<Message>,
    from_bridge: Receiver<Message>,
}

impl CubeManager {
    pub fn new(sender: Sender<Message>, receiver: Receiver<Message>) -> CubeManager {
        CubeManager {
            cubes: HashMap::new(),
            to_bridge: sender,
            from_bridge: receiver,
        }
    }

    pub fn start(&mut self) {
        loop {
            match self.from_bridge.recv().unwrap() {
                Message::Connected(bridge_address, cube_address) => {
                    self.add_new(bridge_address, cube_address);
                }

                Message::SetLampAll(r, g, b) => {
                    println!("SetLampAll: {},{},{}", r, g, b);
                    for cube in self.cubes.values() {
                        println!("cube: {:?}", cube);
                        cube.set_lamp(r, g, b);
                    }
                }

                e @ _ => {
                    eprintln!("Unknown: {:?}", e);
                }
            }
        }
    }

    pub fn add_new(&mut self, bridge_address: String, cube_address: String) {
        println!("add_new: bridge_address={:?}, cube_address={:?}", bridge_address, cube_address);
        let to_bridge = self.to_bridge.clone();
        self.cubes
            .entry(cube_address.clone())
            .and_modify(|cube| cube.bridge = bridge_address.clone())
            .or_insert_with(|| Cube {
                address: cube_address,
                bridge: bridge_address,
                to_bridge: to_bridge,
            });
    }
}
