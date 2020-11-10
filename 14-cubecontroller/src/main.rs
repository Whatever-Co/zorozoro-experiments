mod bridge;
mod bridge_manager;
mod cube;

use bridge_manager::BridgeManager;
use crossbeam_channel::{unbounded, Sender};
use cube::CubeManager;
use iced::{executor, Application, Command, Container, Element, Settings, Subscription, Text};
use iced_native::{input::ButtonState, subscription, Event};
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
    to_cubes_sender: Sender<bridge::Message>,
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
        let (to_cubes_sender, to_cubes_receiver) = unbounded(); // Bridges -> Cubes
        let (to_bridges_sender, to_bridges_receiver) = unbounded(); // Bridges <- Cubes
        let to_cubes2 = to_cubes_sender.clone();
        thread::spawn(move || {
            CubeManager::new(to_bridges_sender, to_cubes_receiver).start();
        });
        thread::spawn(move || {
            BridgeManager::new(to_cubes_sender, to_bridges_receiver).start();
        });
        (
            App {
                key_state: [ButtonState::Released; 256],
                to_cubes_sender: to_cubes2,
            },
            Command::none(),
        )
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
                                println!("sent SetLampAll");
                                self.to_cubes_sender.send(bridge::Message::SetLampAll(255, 0, 0)).unwrap();
                            }
                        }
                    }
                }
                _ => {}
            },
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
