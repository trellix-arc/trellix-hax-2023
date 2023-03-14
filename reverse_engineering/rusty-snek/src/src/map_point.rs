// src/map_point.rs
use crate::travel_direction::SnekDirection;

#[derive(Debug, Copy, Clone, Hash, Eq, PartialEq)]
pub struct MapPoint {
    pub x: u16,
    pub y: u16
}

impl MapPoint {
    pub fn new(x: u16, y: u16) -> Self {
        Self {x, y}
    }

    pub fn transform(&self, travel_direction: SnekDirection, step: u16) -> Self {
        let step = step as i16;
        let transformation = match travel_direction {
            SnekDirection::Up       => (0, -step),
            SnekDirection::Down     => (0, step),
            SnekDirection::Right    => (step, 0),
            SnekDirection::Left     => (-step, 0)
        };

        Self::new(
            Self::transform_value(self.x, transformation.0),
            Self::transform_value(self.y, transformation.1)
        )
    }
    
    fn transform_value(value: u16, by: i16) -> u16 {
        if by.is_negative() && by.abs() as u16 > value {
            panic!("Transforming value {} by {} would result in a negative number", value, by);
        } else {
            (value as i16 + by) as u16
        }
    }
}
