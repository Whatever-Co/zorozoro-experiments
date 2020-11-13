#[macro_use]
extern crate log;

mod bridge;
mod bridge_manager;
mod cube;

use bridge::{IDInfo, Message};
use bridge_manager::BridgeManager;
use crossbeam_channel::{unbounded, Receiver, Sender};
use cube::CubeManager;
use glutin_window::GlutinWindow as Window;
use nalgebra::{Isometry2, Point2, Vector2};
use ncollide2d::pipeline::{CollisionGroups, CollisionObjectSlabHandle, GeometricQueryType};
use ncollide2d::query::Ray;
use ncollide2d::shape::{Cuboid, ShapeHandle};
use ncollide2d::world::CollisionWorld;
use opengl_graphics::{GlGraphics, OpenGL};
use piston::event_loop::{EventSettings, Events};
use piston::input::{Button, ButtonArgs, ButtonEvent, ButtonState, Key, RenderArgs, RenderEvent, UpdateArgs, UpdateEvent};
use piston::window::WindowSettings;
use std::collections::{
    hash_map::Entry::{Occupied, Vacant},
    HashMap,
};
use std::thread;

const DOTS_PER_METER: f64 = 411.0 / 0.560;
const CUBE_SIZE: f64 = 0.0318 * DOTS_PER_METER; // 31.8mm

const COLORS: [[u8; 3]; 3] = [[255, 255, 0], [255, 0, 255], [0, 255, 255]];

struct Cube {
    x: f64,
    y: f64,
    a: f64,
    handle: CollisionObjectSlabHandle,
}

struct App {
    gl: GlGraphics,
    sender: Option<Sender<Message>>,
    receiver: Option<Receiver<Message>>,
    key_state: HashMap<Key, ButtonState>,
    cubes: HashMap<String, Cube>,
    world: CollisionWorld<f32, ()>,
    collision_group: CollisionGroups,
    count: usize,
}

impl App {
    fn new(gl_version: OpenGL) -> App {
        App {
            gl: GlGraphics::new(gl_version),
            sender: None,
            receiver: None,
            key_state: HashMap::with_capacity(256),
            cubes: HashMap::with_capacity(256),
            world: CollisionWorld::new(0.01),
            collision_group: CollisionGroups::new(),
            count: 0,
        }
    }

    fn start(&mut self) {
        let (to_cubes_sender, to_cubes_receiver) = unbounded(); // -> Cubes
        let (to_bridges_sender, to_bridges_receiver) = unbounded(); // -> Bridge
        let (to_ui_sender, to_ui_receiver) = unbounded(); // -> UI
        {
            thread::spawn(move || {
                CubeManager::new(to_bridges_sender, to_cubes_receiver, to_ui_sender).start();
            });
        }
        {
            let to_cubes_sender = to_cubes_sender.clone();
            thread::spawn(move || {
                BridgeManager::new(to_cubes_sender, to_bridges_receiver).start();
            });
        }
        self.sender = Some(to_cubes_sender);
        self.receiver = Some(to_ui_receiver);
    }

    fn input(&mut self, args: &ButtonArgs) {
        match args.button {
            Button::Keyboard(key) => match self.key_state.entry(key) {
                Occupied(mut entry) => {
                    let prev = entry.insert(args.state);
                    if prev != args.state && args.state == ButtonState::Press {
                        self.on_press(&key);
                    }
                }
                Vacant(entry) => {
                    entry.insert(args.state);
                    if args.state == ButtonState::Press {
                        self.on_press(&key);
                    }
                }
            },
            _ => (),
        }
    }

    fn on_press(&mut self, key: &Key) {
        trace!("on_press: {:?}", key);
        match key {
            Key::D1 => {
                let [r, g, b] = COLORS[self.count % COLORS.len()];
                self.count = self.count + 1;
                self.sender.as_ref().unwrap().try_send(Message::SetLampAll(r, g, b)).unwrap();
            }
            Key::D2 => {
                let angle = (self.count * 135) % 360;
                self.count = self.count + 1;
                self.sender.as_ref().unwrap().try_send(Message::SetDirectionAll(angle as u16)).unwrap();
            }
            Key::D3 => {
                self.sender.as_ref().unwrap().try_send(Message::StartGoAround).unwrap();
            }
            _ => (),
        }
    }

    fn update(&mut self, _args: &UpdateArgs) {
        for message in self.receiver.as_mut().unwrap().try_iter() {
            match message {
                Message::Connected(_, cube_address) => match self.cubes.entry(cube_address) {
                    Occupied(_) => (),
                    Vacant(entry) => {
                        let size = (CUBE_SIZE / 2.0) as f32;
                        let (handle, _) = self.world.add(
                            Isometry2::new(Vector2::new(0.0, 0.0), 0.0),
                            ShapeHandle::new(Cuboid::new(Vector2::new(size, size))),
                            self.collision_group,
                            GeometricQueryType::Contacts(0.0, 0.0),
                            (),
                        );
                        entry.insert(Cube {
                            x: 0.0,
                            y: 0.0,
                            a: 0.0,
                            handle,
                        });
                    }
                },

                Message::Disconnected(_, cube_address) => {
                    if let Some(cube) = self.cubes.remove(&cube_address) {
                        self.world.remove(&[cube.handle]);
                    }
                }

                Message::IDInfo(cube_address, id_info) => match id_info {
                    IDInfo::PositionID(x, y, a) => match self.cubes.entry(cube_address) {
                        Occupied(mut entry) => {
                            let cube = entry.get_mut();
                            cube.x = From::from(x);
                            cube.y = From::from(y);
                            cube.a = From::from(a);
                            if let Some(object) = self.world.get_mut(cube.handle) {
                                object.set_position(Isometry2::new(Vector2::new(x.into(), y.into()), ((a + 270) as f32).to_radians()));
                            }
                        }
                        Vacant(_) => (),
                    },
                    _ => (),
                },

                _ => (),
            }
        }

        self.world.update();

        // for (_, cube) in &self.cubes {
        //     if let Some(obj) = self.world.get_mut(cube.handle) {
        //         let p = obj.position() * Point2::new(0.0, -(CUBE_SIZE / 1.99) as f32);
        //         let v = Vector2::y();
        //         let ray = Ray::new(p, v);
        //         if let Some(hit) = self.world.first_interference_with_ray(&ray, 100.0, &self.collision_group) {
        //             trace!("hit:{:?}", hit.inter);
        //         }
        //     }
        // }
    }

    fn render(&mut self, args: &RenderArgs) {
        let context = self.gl.draw_begin(args.viewport());

        use graphics::*;

        const TOIO_BLUE: [f32; 4] = [0.000, 0.684, 0.792, 1.0];
        const WHITE: [f32; 4] = [1.0, 1.0, 1.0, 1.0];
        const RED: [f32; 4] = [1.0, 0.0, 0.0, 1.0];
        const GREEN: [f32; 4] = [0.0, 1.0, 0.0, 1.0];

        let gl = &mut self.gl;

        clear(TOIO_BLUE, gl);

        let square = rectangle::centered_square(0.0, 0.0, CUBE_SIZE / 2.0);
        for (_, cube) in &self.cubes {
            let transform = context.transform.trans(cube.x, cube.y).rot_deg(cube.a - 90.0);
            rectangle(WHITE, square, transform, gl);

            if let Some(obj) = self.world.get_mut(cube.handle) {
                let a = (CUBE_SIZE / 2.0) as f32;
                let len = (0.030 * DOTS_PER_METER) as f32; // forward 30mm
                let ray = Ray::new(Point2::new(0.0, a + 1.0), Vector2::new(0.0, 1.0)).transform_by(obj.position());
                let color = match self.world.first_interference_with_ray(&ray, len, &self.collision_group) {
                    Some(hit) => {
                        trace!("hit={:?}", hit.inter);
                        RED
                    }
                    None => GREEN,
                };
                let start: [f64; 2] = [ray.origin.x.into(), ray.origin.y.into()];
                let end = ray.origin + ray.dir * len;
                let end: [f64; 2] = [end.x.into(), end.y.into()];
                line_from_to(color, 1.0, start, end, context.transform, gl);
            }
        }

        self.gl.draw_end();
    }
}

fn main() {
    env_logger::init();
    info!("Starting up!");

    let gl_version = OpenGL::V3_3;
    let mut window: Window = WindowSettings::new("Cube Controller", [800, 600])
        .graphics_api(gl_version)
        .samples(4)
        .vsync(true)
        .exit_on_esc(true)
        .build()
        .unwrap();

    let mut app = App::new(gl_version);
    app.start();

    let mut events = Events::new(EventSettings::new());
    while let Some(e) = events.next(&mut window) {
        // trace!("{:?}", e);
        if let Some(args) = e.button_args() {
            app.input(&args);
        }
        if let Some(args) = e.update_args() {
            app.update(&args);
        }
        if let Some(args) = e.render_args() {
            app.render(&args);
        }
    }

    info!("Done!");
}
