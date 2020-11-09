use std::collections::HashMap;

pub struct Cube {
    address: String,
    bridge: String,
}

pub struct CubeManager {
    cubes: HashMap<String, Cube>,
}

impl CubeManager {
    pub fn new() -> CubeManager {
        CubeManager { cubes: HashMap::new() }
    }

    pub fn add_new(&mut self, bridge_address: String, cube_address: String) {
        match self.cubes.get_mut(&cube_address) {
            Some(cube) => {
                cube.bridge = bridge_address.clone();
            }
            None => {
                let cube = Cube {
                    address: cube_address.clone(),
                    bridge: bridge_address,
                };
                self.cubes.insert(cube_address, cube);
            }
        }
    }
}
