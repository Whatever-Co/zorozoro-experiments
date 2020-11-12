mod bridge;
mod bridge_manager;
mod cube;

use bridge::{IDInfo, Message};
use bridge_manager::BridgeManager;
use crossbeam_channel::{unbounded, Receiver};
use cube::CubeManager;
use glutin_window::GlutinWindow as Window;
use opengl_graphics::{GlGraphics, OpenGL};
use piston::event_loop::{EventSettings, Events};
use piston::input::{RenderArgs, RenderEvent, UpdateArgs, UpdateEvent};
use piston::window::WindowSettings;
use std::collections::HashMap;
use std::thread;

#[derive(Debug, Default)]
struct Cube {
    x: f64,
    y: f64,
    a: f64,
}

struct App {
    gl: GlGraphics,
    receiver: Option<Receiver<Message>>,
    cubes: HashMap<String, Cube>,
}

impl App {
    fn new(gl_version: OpenGL) -> App {
        App {
            gl: GlGraphics::new(gl_version),
            receiver: None,
            cubes: HashMap::with_capacity(256),
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
        self.receiver = Some(to_ui_receiver);
    }

    fn update(&mut self, _args: &UpdateArgs) {
        if let Some(receiver) = &self.receiver {
            for message in receiver.try_iter() {
                match message {
                    Message::Connected(_, cube_address) => {
                        self.cubes.entry(cube_address.clone()).or_insert(Default::default());
                    }

                    Message::Disconnected(_, cube_address) => {
                        self.cubes.remove(&cube_address);
                    }

                    Message::IDInfo(cube_address, id_info) => match id_info {
                        IDInfo::PositionID(x, y, a) => {
                            self.cubes.entry(cube_address).and_modify(|cube| {
                                cube.x = From::from(x);
                                cube.y = From::from(y);
                                cube.a = From::from(a);
                                println!("cube={:?}", cube);
                            });
                        }
                        _ => (),
                    },

                    _ => (),
                }
            }
        }
    }

    fn render(&mut self, args: &RenderArgs) {
        let context = self.gl.draw_begin(args.viewport());

        use graphics::*;

        const TOIO_BLUE: [f32; 4] = [0.000, 0.684, 0.792, 1.0];
        const WHITE: [f32; 4] = [1.0, 1.0, 1.0, 1.0];

        let gl = &mut self.gl;

        clear(TOIO_BLUE, gl);

        let square = rectangle::centered_square(0.0, 0.0, 15.0);
        for (_, cube) in &self.cubes {
            let transform = context.transform.trans(cube.x, cube.y).rot_deg(cube.a);
            rectangle(WHITE, square, transform, gl);
        }

        self.gl.draw_end();
    }
}

fn main() {
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
        if let Some(args) = e.update_args() {
            app.update(&args);
        }
        if let Some(args) = e.render_args() {
            app.render(&args);
        }
    }
}
