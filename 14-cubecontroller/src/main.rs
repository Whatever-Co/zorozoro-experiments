mod bridge;
mod cube;

use bridge::BridgeManager;
use cube::CubeManager;
use std::sync::{Arc, Mutex};
use std::{thread, time};

fn main() {
    let cube_manager = Arc::new(Mutex::new(CubeManager::new()));

    thread::spawn(move || {
        let cube_manager = cube_manager.clone();
        BridgeManager::new(cube_manager).start();
    });

    loop {
        thread::sleep(time::Duration::from_millis(10));
    }
}
