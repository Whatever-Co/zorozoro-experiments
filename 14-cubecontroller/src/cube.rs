use crate::bridge::{IDInfo, Message};
use crossbeam_channel::{Receiver, Sender};
use nalgebra::Vector2;
use std::collections::{
    hash_map::Entry::{Occupied, Vacant},
    HashMap,
};
use std::{
    thread,
    time::{Duration, Instant},
};

#[derive(Debug)]
pub struct Cube {
    position: Vector2<f32>,
    rotation: f32,
    address: String,
    bridge: String,
    to_bridge: Sender<Message>,
    battery: u8,
    going_around: bool,
    last_move: Instant,
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

    pub fn set_direction(&self, angle: u16) {
        self.to_bridge
            .send(Message::SetDirection(self.address.clone(), self.bridge.clone(), angle))
            .unwrap();
    }

    pub fn start_go_around(&mut self) {
        self.going_around = true;
        self.last_move = Instant::now();
    }

    fn send_next_move(&mut self) {
        let mat_min = Vector2::new(98.0, 142.0);
        let mat_max = Vector2::new(402.0, 358.0);
        let mat_center = (mat_min + mat_max) / 2.0;
        const MIN_RADIUS: f32 = 50.0;
        const MAX_RADIUS: f32 = 100.0;
        const SPACING: f32 = 35.0;
        let p = mat_center - self.position;
        let r = nalgebra::clamp(p.norm(), MIN_RADIUS, MAX_RADIUS);
        let radius = ((r - MIN_RADIUS) / SPACING).round() * SPACING + MIN_RADIUS;
        let start_angle = p.y.atan2(p.x);
        const DISTANCE: f32 = 50.0;
        let C = 2.0 * radius * std::f32::consts::PI;
        let end_angle = start_angle + DISTANCE / C * std::f32::consts::TAU;
        let target_x = -end_angle.cos() * radius + mat_center.x;
        let target_y = -end_angle.sin() * radius + mat_center.y;
        trace!(
            "radius={:?}, C={}, start_angle={:?}, end_angle={:?}, x={:?}, y={:?}",
            radius,
            C,
            start_angle,
            end_angle,
            target_x,
            target_y,
        );
        self.to_bridge
            .try_send(Message::MoveToTarget(
                self.address.clone(),
                self.bridge.clone(),
                target_x as u16,
                target_y as u16,
            ))
            .unwrap();
    }

    pub fn tick(&mut self) {
        if self.going_around {
            let now = Instant::now();
            if (now - self.last_move).as_millis() >= 500 {
                self.send_next_move();
                self.last_move = now;
            }
        }
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
            while let Ok(message) = self.from_bridge.try_recv() {
                self.process_message(&message);
            }

            for cube in self.cubes.values_mut() {
                cube.tick();
            }

            thread::sleep(Duration::from_millis(50));
        }
    }

    fn process_message(&mut self, message: &Message) {
        match message {
            Message::Connected(bridge_address, cube_address) => {
                self.add_new(bridge_address.clone(), cube_address.clone());
                self.to_ui.try_send(message.clone()).unwrap();
            }

            Message::Disconnected(_bridge_address, cube_address) => {
                self.cubes.remove(cube_address);
                self.to_ui.try_send(message.clone()).unwrap();
            }

            Message::BatteryInfo(cube_address, value) => {
                self.cubes.entry(cube_address.clone()).and_modify(|cube| cube.set_battery(*value));
            }

            Message::IDInfo(cube_address, id_info) => {
                match id_info {
                    IDInfo::PositionID(x, y, a) => match self.cubes.entry(cube_address.to_string()) {
                        Occupied(mut entry) => {
                            let mut cube = entry.get_mut();
                            cube.position = Vector2::new(*x as f32, *y as f32);
                            cube.rotation = *a as f32;
                        }
                        Vacant(_) => (),
                    },
                    _ => (),
                }
                self.to_ui.try_send(message.clone()).unwrap();
            }

            Message::SetLampAll(r, g, b) => {
                for cube in self.cubes.values() {
                    cube.set_lamp(*r, *g, *b);
                }
            }

            Message::SetDirectionAll(angle) => {
                for cube in self.cubes.values() {
                    cube.set_direction(*angle);
                }
            }

            Message::StartGoAround => {
                for cube in self.cubes.values_mut() {
                    cube.start_go_around();
                }
            }

            e @ _ => {
                error!("Unhandled message: {:?}", e);
            }
        }
    }

    pub fn add_new(&mut self, bridge_address: String, cube_address: String) {
        trace!("add_new: bridge_address={:?}, cube_address={:?}", bridge_address, cube_address);
        let to_bridge = self.to_bridge.clone();
        self.cubes
            .entry(cube_address.clone())
            .and_modify(|cube| cube.bridge = bridge_address.clone())
            .or_insert_with(|| Cube {
                position: Vector2::new(0.0, 0.0),
                rotation: 0.0,
                address: cube_address,
                bridge: bridge_address,
                to_bridge: to_bridge,
                battery: 0,
                going_around: true,
                last_move: Instant::now(),
            });
    }
}
