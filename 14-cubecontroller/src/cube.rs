use crate::bridge::Message;
use crossbeam_channel::{Receiver, Sender};
use std::collections::HashMap;

#[derive(Debug)]
pub struct Cube {
    address: String,
    bridge: String,
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

    pub fn add_new(&mut self, bridge_address: String, cube_address: String) {
        match self.cubes.get_mut(&cube_address) {
            Some(cube) => {
                cube.bridge = bridge_address.clone();
            }
            None => {
                let cube = Cube {
                    address: cube_address.clone(),
                    bridge: bridge_address,
                };
                self.cubes.insert(cube_address, cube);
            }
        }
    }

    pub fn set_lamp_all(&mut self) {}
}
