// src/io.rs

use crate::travel_direction::SnekDirection;

pub enum Command {
    Quit,
    Turn(SnekDirection)
}
