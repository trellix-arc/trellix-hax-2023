mod snek;
mod travel_direction;
mod game;
mod map_point;
mod io;

use crate::game::Game;
use std::io::stdout;

fn main() {
    println!("Hello, world!");
    Game::new(stdout(), 29, 20).run();
}
