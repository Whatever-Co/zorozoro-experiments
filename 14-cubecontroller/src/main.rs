mod bridge;
mod bridge_manager;
mod cube;

use bridge::Message;
use bridge_manager::BridgeManager;
use crossbeam_channel::{unbounded, Receiver};
use cube::CubeManager;
use glutin_window::GlutinWindow as Window;
use opengl_graphics::{GlGraphics, OpenGL};
use piston::event_loop::{EventSettings, Events};
use piston::input::{RenderArgs, RenderEvent, UpdateArgs, UpdateEvent};
use piston::window::WindowSettings;
use std::thread;

struct App {
    gl: GlGraphics,
    receiver: Option<Receiver<Message>>,
}

impl App {
    fn new(gl_version: OpenGL) -> App {
        App {
            gl: GlGraphics::new(gl_version),
            receiver: None,
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

    fn update(&self, _args: &UpdateArgs) {
        if let Some(receiver) = &self.receiver {
            for message in receiver.try_iter() {
                println!("message={:?}", message);
            }
        }
    }

    fn render(&mut self, args: &RenderArgs) {
        self.gl.draw(args.viewport(), |_context, gl| {
            graphics::clear([0.000, 0.684, 0.792, 1.0], gl);
        });
    }
}

fn main() {
    let gl_version = OpenGL::V3_3;
    let mut window: Window = WindowSettings::new("Cube Controller", [800, 600])
        .graphics_api(gl_version)
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
