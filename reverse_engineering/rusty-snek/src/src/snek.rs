// src/snek.rs

use crate::travel_direction::SnekDirection;
use crate::map_point::MapPoint;

#[derive(Debug)]
pub struct Snek {
    body: Vec<MapPoint>,
    direction: SnekDirection,
    digesting: bool
}

impl Snek {
    pub fn new(start: MapPoint, length: u16, direction: SnekDirection) -> Self {
        let opposite = direction.opposite();
        let body: Vec<MapPoint> = (0..length)
            .into_iter()
            .map(|i| start.transform(opposite, i))
            .collect();
        Self {body, direction, digesting: false}
    }

    pub fn get_head(&self) -> MapPoint {
        self.body.first().unwrap().clone()
    }

    pub fn get_body(&self) -> Vec<MapPoint> {
        self.body.clone()
    }

    pub fn get_direction(&self) -> SnekDirection {
        self.direction.clone()
    }

    pub fn contains_point(&self, point: &MapPoint) -> bool {
        self.body.contains(point)
    }

    pub fn slither(&mut self) {
        self.body.insert(0, self.body.first().unwrap().transform(self.direction, 1));
        if !self.digesting {
            self.body.remove(self.body.len() -1);
        } else {
            self.digesting = false;
        }
    }

    pub fn set_direction(&mut self, direction: SnekDirection) {
        self.direction = direction;
    }

    pub fn grow(&mut self) {
        self.digesting = true;
    }
}
