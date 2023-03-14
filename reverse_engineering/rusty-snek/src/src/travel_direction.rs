// src/travel_direction.rs

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum SnekDirection {
    Up,
    Down,
    Right,
    Left
}

impl SnekDirection {
    pub fn opposite(&self) -> Self {
        match self {
            Self:: Up       => Self::Down,
            Self::Down      => Self::Up,
            Self::Right     => Self::Left,
            Self::Left      => Self::Right
        }
    }
}
