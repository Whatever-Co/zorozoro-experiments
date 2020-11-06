mod bridge;
use bridge::BridgeManager;

fn main() {
    BridgeManager::spawn();

    loop {}
}
