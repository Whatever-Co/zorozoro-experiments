mod bridge;
mod cube;

use bridge::BridgeManager;
use cube::CubeManager;
use iced::{executor, Application, Command, Container, Element, Settings, Subscription, Text};
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let cube_manager = Arc::new(Mutex::new(CubeManager::new()));

    thread::spawn(move || {
        let cube_manager = cube_manager.clone();
        BridgeManager::new(cube_manager).start();
    });

    App::run(Settings {
        window: iced::window::Settings {
            size: (400, 300),
            resizable: false,
            ..iced::window::Settings::default()
        },
        ..Settings::default()
    });
}

#[derive(Debug, Default)]
struct App {}

#[derive(Debug)]
pub enum Message {
    EventOccurred(iced_native::Event),
}

impl Application for App {
    type Executor = executor::Default;
    type Message = Message;
    type Flags = ();

    fn new(_flags: ()) -> (Self, Command<Message>) {
        (Self::default(), Command::none())
    }

    fn title(&self) -> String {
        String::from("Cube controller")
    }

    fn update(&mut self, message: Message) -> Command<Message> {
        match message {
            Message::EventOccurred(event) => {
                println!("Event={:?}", event);
            }
            _ => println!("?"),
        }
        Command::none()
    }

    fn subscription(&self) -> Subscription<Message> {
        iced_native::subscription::events().map(Message::EventOccurred)
    }

    fn view(&mut self) -> Element<Message> {
        Container::new(Text::new("aaa")).into()
    }
}
