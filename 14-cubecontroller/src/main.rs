mod bridge;
mod bridge_manager;
mod cube;

use bridge_manager::BridgeManager;
use core::pin::Pin;
use crossbeam_channel::{unbounded, Receiver, Sender};
use cube::CubeManager;
use iced::{
    executor,
    futures::{
        stream::BoxStream,
        task::{Context, Poll},
        StreamExt,
    },
    Application, Command, Container, Element, Settings, Subscription, Text,
};
use iced_native::{input::ButtonState, subscription, Event};
use std::sync::Arc;
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
enum State {
    Initial,
    Running,
}

#[derive(Debug)]
struct App {
    state: State,
    key_state: [ButtonState; 256],
    to_cubes_sender: Sender<bridge::Message>,
    to_ui_receiver: Arc<iced::futures::channel::mpsc::Receiver<bridge::Message>>,
}

#[derive(Debug)]
pub enum Message {
    Ready(()),
    EventOccurred(Event),
    Hoge,
}

impl Application for App {
    type Executor = executor::Default;
    type Message = Message;
    type Flags = ();

    fn new(_flags: ()) -> (Self, Command<Message>) {
        let (to_cubes_sender, to_cubes_receiver) = unbounded(); // -> Cubes
        let (to_bridges_sender, to_bridges_receiver) = unbounded(); // -> Bridge
        let (to_ui_sender, to_ui_receiver) = iced::futures::channel::mpsc::channel(10000); // -> UI
        {
            let to_ui_sender = to_ui_sender.clone();
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
        (
            App {
                state: State::Initial,
                key_state: [ButtonState::Released; 256],
                to_cubes_sender,
                to_ui_receiver: Arc::new(to_ui_receiver),
            },
            Command::perform(async {}, Message::Ready),
        )
    }

    fn title(&self) -> String {
        String::from("Cube controller")
    }

    fn update(&mut self, message: Message) -> Command<Message> {
        use iced_native::input::keyboard::{Event::Input, KeyCode};
        println!("update: {:?}", message);
        match message {
            Message::Ready(_) => self.state = State::Running,
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
            Message::Hoge => println!("HOGE!!!"),
        }
        Command::none()
    }

    fn subscription(&self) -> Subscription<Message> {
        println!("subscription: {:?}", self.state);
        match self.state {
            State::Initial => Subscription::from_recipe(ReceiverRecipe {
                receiver: self.to_ui_receiver.clone(),
            }),
            State::Running => subscription::events().map(Message::EventOccurred),
        }
    }

    fn view(&mut self) -> Element<Message> {
        Container::new(Text::new("aaa")).into()
    }
}

struct ReceiverRecipe {
    receiver: Arc<iced::futures::channel::mpsc::Receiver<bridge::Message>>,
}

impl<H, I> iced_native::subscription::Recipe<H, I> for ReceiverRecipe
where
    H: std::hash::Hasher,
{
    type Output = Message;

    fn hash(&self, _state: &mut H) {
        // use std::hash::Hash;
        // std::any::TypeId::of::<Self>().hash(state);
        // self.receiver.hash(state);
    }

    fn stream(self: Box<Self>, _: BoxStream<'static, I>) -> BoxStream<'static, Self::Output> {
        self.receiver.map(|_| Message::Hoge).boxed()
    }
}
