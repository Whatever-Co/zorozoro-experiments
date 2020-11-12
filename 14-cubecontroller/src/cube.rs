use crate::bridge::Message;
use crossbeam_channel::{Receiver, Sender};
use std::collections::HashMap;

#[derive(Debug)]
pub struct Cube {
    address: String,
    bridge: String,
    to_bridge: Sender<Message>,
    battery: u8,
}

impl Cube {
    pub fn set_lamp(&self, red: u8, green: u8, blue: u8) {
        self.to_bridge
            .send(Message::SetLamp(self.address.clone(), self.bridge.clone(), red, green, blue))
            .unwrap();
    }

    pub fn set_battery(&mut self, value: u8) {
        if value == self.battery {
            return;
        }
        self.battery = value;
        let (r, g, b) = match self.battery {
            0..=10 => (255, 0, 0),
            11..=20 => (255, 0, 0),
            21..=50 => (254, 176, 25),
            _ => (0, 255, 0),
        };
        self.to_bridge
            .send(Message::SetLamp(self.address.clone(), self.bridge.clone(), r, g, b))
            .unwrap();
    }
}

#[derive(Debug)]
pub struct CubeManager {
    cubes: HashMap<String, Cube>,
    to_bridge: Sender<Message>,
    from_bridge: Receiver<Message>,
    to_ui: Sender<Message>,
}

impl CubeManager {
    pub fn new(to_bridge: Sender<Message>, from_bridge: Receiver<Message>, to_ui: Sender<Message>) -> CubeManager {
        CubeManager {
            cubes: HashMap::new(),
            to_bridge,
            from_bridge,
            to_ui,
        }
    }

    pub fn start(&mut self) {
        loop {
            match self.from_bridge.recv().unwrap() {
                Message::Connected(bridge_address, cube_address) => {
                    self.add_new(bridge_address.clone(), cube_address.clone());
                    self.to_ui.try_send(Message::Connected(bridge_address, cube_address)).unwrap();
                }

                Message::Disconnected(bridge_address, cube_address) => {
                    self.cubes.remove(&cube_address);
                    self.to_ui.try_send(Message::Disconnected(bridge_address, cube_address)).unwrap();
                }

                Message::BatteryInfo(cube_address, value) => {
                    self.cubes.entry(cube_address.clone()).and_modify(|cube| cube.set_battery(value));
                }

                Message::IDInfo(cube_address, value) => {
                    self.to_ui.try_send(Message::IDInfo(cube_address, value)).unwrap();
                }

                Message::SetLampAll(r, g, b) => {
                    for cube in self.cubes.values() {
                        cube.set_lamp(r, g, b);
                    }
                }

                e @ _ => {
                    eprintln!("Unhandled message: {:?}", e);
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
                battery: 0,
            });
    }
}
