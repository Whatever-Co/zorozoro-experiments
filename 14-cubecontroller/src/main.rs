mod bridge;
mod bridge_manager;
mod cube;

use bridge_manager::BridgeManager;
use crossbeam_channel::unbounded;
use cube::CubeManager;
use std::{thread, time::Duration};

fn main() {
    let (to_cubes_sender, to_cubes_receiver) = unbounded(); // -> Cubes
    let (to_bridges_sender, to_bridges_receiver) = unbounded(); // -> Bridge
    {
        thread::spawn(move || {
            CubeManager::new(to_bridges_sender, to_cubes_receiver).start();
        });
    }
    {
        let to_cubes_sender = to_cubes_sender.clone();
        thread::spawn(move || {
            BridgeManager::new(to_cubes_sender, to_bridges_receiver).start();
        });
    }

    loop {
        thread::sleep(Duration::from_millis(100));
    }
}
