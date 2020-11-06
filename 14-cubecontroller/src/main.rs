#[macro_use]
extern crate lazy_static;

mod bridge;
use bridge::BridgeManager;

fn main() {
    BridgeManager::start();
    loop {}
}
