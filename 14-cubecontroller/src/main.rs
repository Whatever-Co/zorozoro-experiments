mod bridge;
mod bridge_manager;
mod cube;

use bridge_manager::BridgeManager;
use cube::CubeManager;
use iced::{executor, Application, Command, Container, Element, Settings, Subscription, Text};
use iced_native::{input::ButtonState, subscription, Event};
use std::sync::mpsc::channel;
use std::thread;

fn main() {
    App::run(Settings {
        window: iced::window::Settings {
            size: (400, 300),
            resizable: false,
            ..iced::window::Settings::default()
        },
        ..Settings::default()
    });
}

#[derive(Debug)]
struct App {
    key_state: [ButtonState; 256],
    cube_manager: CubeManager,
}

#[derive(Debug)]
pub enum Message {
    EventOccurred(Event),
}

impl Application for App {
    type Executor = executor::Default;
    type Message = Message;
    type Flags = ();

    fn new(_flags: ()) -> (Self, Command<Message>) {
        // Bridge -> Cubes
        let (b2c_sender, b2c_receiver) = channel();
        // Bridge <- Cubes
        let (c2b_sender, c2b_receiver) = channel();
        let app = App {
            key_state: [ButtonState::Released; 256],
            cube_manager: CubeManager::new(c2b_sender, b2c_receiver),
        };
        thread::spawn(move || {
            BridgeManager::new(b2c_sender, c2b_receiver).start();
        });
        (app, Command::none())
    }

    fn title(&self) -> String {
        String::from("Cube controller")
    }

    fn update(&mut self, message: Message) -> Command<Message> {
        use iced_native::input::keyboard::{Event::Input, KeyCode};
        match message {
            Message::EventOccurred(event) => match event {
                Event::Keyboard(event) => {
                    if let Input {
                        state,
                        key_code,
                        modifiers: _,
                    } = event
                    {
                        let index = key_code as usize;
                        let previous = self.key_state[index];
                        if previous != state {
                            self.key_state[index] = state;
                            println!("state={:?}, key_code={:?}", state, key_code);
                            if state == ButtonState::Pressed && key_code == KeyCode::E {
                                self.cube_manager.set_lamp_all();
                            }
                        }
                    }
                }
                _ => {}
            },
            _ => println!("?"),
        }
        Command::none()
    }

    fn subscription(&self) -> Subscription<Message> {
        subscription::events().map(Message::EventOccurred)
    }

    fn view(&mut self) -> Element<Message> {
        Container::new(Text::new("aaa")).into()
    }
}

impl App {}
