#[macro_use]
extern crate log;

mod bridge;
mod bridge_manager;
mod cube;

use bridge::{IDInfo, Message};
use bridge_manager::BridgeManager;
use crossbeam_channel::{unbounded, Receiver, Sender};
use cube::CubeManager;
use env_logger::{Builder, Env};
use glutin_window::GlutinWindow as Window;
use opengl_graphics::{GlGraphics, GlyphCache, OpenGL, TextureSettings};
use piston::event_loop::{EventSettings, Events};
use piston::input::{Button, ButtonArgs, ButtonEvent, ButtonState, Key, RenderArgs, RenderEvent, UpdateArgs, UpdateEvent};
use piston::window::WindowSettings;
use std::collections::hash_map::Entry::{Occupied, Vacant};
use std::collections::HashMap;
use std::thread;

const DOTS_PER_METER: f64 = 411.0 / 0.560;
const CUBE_SIZE: f64 = 0.0318 * DOTS_PER_METER; // 31.8mm

const COLORS: [[u8; 3]; 3] = [[255, 255, 0], [255, 0, 255], [0, 255, 255]];

struct Cube {
    x: f64,
    y: f64,
    a: f64,
    hit: bool,
}

struct App<'a> {
    gl: GlGraphics,
    sender: Option<Sender<Message>>,
    receiver: Option<Receiver<Message>>,
    key_state: HashMap<Key, ButtonState>,
    cubes: HashMap<String, Cube>,
    count: usize,
    glyph_cache: GlyphCache<'a>,
}

impl App<'_> {
    fn new(gl_version: OpenGL) -> App<'static> {
        App {
            gl: GlGraphics::new(gl_version),
            sender: None,
            receiver: None,
            key_state: HashMap::with_capacity(256),
            cubes: HashMap::with_capacity(256),
            count: 0,
            glyph_cache: GlyphCache::new("assets/RobotoCondensed-Regular.ttf", (), TextureSettings::new()).unwrap(),
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
                self.sender.as_ref().unwrap().try_send(Message::ShowBatteryInfoAll).unwrap();
            }
            Key::D2 => {
                let [r, g, b] = COLORS[self.count % COLORS.len()];
                self.count = self.count + 1;
                self.sender.as_ref().unwrap().try_send(Message::SetLampAll(r, g, b)).unwrap();
            }
            Key::D3 => {
                let angle = (self.count * 135) % 360;
                self.count = self.count + 1;
                self.sender.as_ref().unwrap().try_send(Message::SetDirectionAll(angle as u16)).unwrap();
            }
            Key::D4 => {
                self.sender.as_ref().unwrap().try_send(Message::LookCenterAll).unwrap();
            }
            Key::D5 => {
                self.sender.as_ref().unwrap().try_send(Message::StartGoAround).unwrap();
            }
            Key::D6 => {
                self.sender.as_ref().unwrap().try_send(Message::StopAll).unwrap();
            }
            _ => (),
        }
    }

    fn update(&mut self, _args: &UpdateArgs) {
        for message in self.receiver.as_mut().unwrap().try_iter() {
            match message {
                Message::Connected(_, cube_address) => {
                    self.cubes.entry(cube_address).or_insert_with(|| Cube {
                        x: -999.9,
                        y: -999.9,
                        a: 0.0,
                        hit: false,
                    });
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
                        });
                    }
                    IDInfo::PositionIDMissed => {
                        self.cubes.entry(cube_address).and_modify(|cube| {
                            cube.x = -999.9;
                            cube.y = -999.9;
                            cube.a = 0.0;
                        });
                    }
                    _ => (),
                },

                Message::HitStateChanged(cube_address, hit) => {
                    debug!("Hit state changed: {:?}, {:?}", cube_address, hit);
                    self.cubes.entry(cube_address).and_modify(|cube| cube.hit = hit);
                }

                _ => (),
            }
        }
    }

    fn render(&mut self, args: &RenderArgs) {
        use graphics::*;

        const TOIO_BLUE: [f32; 4] = [0.000, 0.684, 0.792, 1.0];
        const WHITE: [f32; 4] = [1.0, 1.0, 1.0, 1.0];
        const RED: [f32; 4] = [1.0, 0.0, 0.0, 1.0];
        const GREEN: [f32; 4] = [0.0, 1.0, 0.0, 1.0];

        let context = self.gl.draw_begin(args.viewport());
        let gl = &mut self.gl;

        clear(TOIO_BLUE, gl);

        let square = rectangle::centered_square(0.0, 0.0, CUBE_SIZE / 2.0);
        for cube in self.cubes.values() {
            let transform = context.transform.trans(cube.x, cube.y).rot_deg(cube.a - 90.0);
            rectangle(WHITE, square, transform, gl);

            let a = CUBE_SIZE / 2.0;
            let color = if cube.hit { RED } else { GREEN };
            let start: [f64; 2] = [0.0, a + 1.0];
            let end: [f64; 2] = [0.0, start[1] + cube::HIT_LEN];
            line_from_to(color, 1.0, start, end, transform, gl);
        }

        self.draw_text(&context, 10.0, 40.0, 30, &WHITE, "ZOROZORO Controller");
        // self.draw_text(&context, 10.0, 70.0, 18, &WHITE, &format!("Bridges: {}", self.cubes.len()));
        self.draw_text(&context, 10.0, 70.0, 18, &WHITE, &format!("Cubes: {}", self.cubes.len()));
        {
            const MENU_TEXT: &str = "1. Battery Status\n2. Random Color\n3. Random Rotate\n4. Look Center\n5. Go Around\n6. Stop\n0. Shutdown";
            let lines = MENU_TEXT.split("\n").collect::<Vec<_>>();
            let mut y = args.viewport().window_size[1] - 25.0 * (lines.len() as f64) - 10.0;
            for line in lines.iter() {
                y += 25.0;
                self.draw_text(&context, 10.0, y, 18, &WHITE, line);
            }
        }

        self.gl.draw_end();
    }

    fn draw_text(&mut self, context: &graphics::Context, x: f64, y: f64, size: u32, color: &[f32; 4], s: &str) {
        use graphics::*;
        let gl = &mut self.gl;
        let glyph_cache = &mut self.glyph_cache;
        text::Text::new_color(*color, size)
            .draw(s, glyph_cache, &DrawState::default(), context.transform.trans(x, y), gl)
            .unwrap();
    }
}

fn main() {
    Builder::from_env(Env::default()).format_timestamp_millis().init();
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
