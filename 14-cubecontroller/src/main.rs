mod bridge;
mod cube;

use bridge::BridgeManager;
use cube::CubeManager;
use iced::{executor, Application, Command, Container, Element, Settings, Subscription, Text};
use iced_native::{input::ButtonState, subscription, Event};
use std::sync::{Arc, Mutex};
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
    cube_manager: Arc<Mutex<CubeManager>>,
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
        let app = App {
            key_state: [ButtonState::Released; 256],
            cube_manager: Arc::new(Mutex::new(CubeManager::new())),
        };
        app.start();
        (app, Command::none())
    }

    fn title(&self) -> String {
        String::from("Cube controller")
    }

    fn update(&mut self, message: Message) -> Command<Message> {
        match message {
            Message::EventOccurred(event) => match event {
                Event::Keyboard(event) => {
                    if let iced_native::input::keyboard::Event::Input {
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

impl App {
    fn start(&self) {
        let cube_manager = self.cube_manager.clone();
        thread::spawn(move || {
            BridgeManager::new(cube_manager).start();
        });
    }
}
