// src/game.rs

use crate::snek::Snek;
use crate::map_point::MapPoint;
use crate::travel_direction::SnekDirection;
use std::io::Stdout;
use crossterm::{ExecutableCommand};
use crossterm::terminal::{Clear, ClearType, size, SetSize, enable_raw_mode, disable_raw_mode};
use crossterm::style::{SetForegroundColor, Print, ResetColor, Color};
use std::time::{Duration, Instant};
use crossterm::cursor::{Show, MoveTo, Hide};
use crossterm::event::{poll, read, Event, KeyCode, KeyModifiers, KeyEvent};
use crate::io::Command;
use rand::Rng;

const MAX_INTERVAL: u16 = 200;
const MIN_INTERVAL: u16 = 5;
const MAX_SPEED:    u16 = 25;

#[derive(Debug)]
pub struct Game {
    stdout: Stdout,
    original_term_size: (u16,u16),
    width: u16,
    height: u16,
    food: Option<MapPoint>,
    snek: Snek,
    speed: u16,
    score: u16,
    max_score: usize,
    collected_food: Vec<MapPoint>,
    flag: Vec<MapPoint>
}

impl Game {
    pub fn new(stdout: Stdout, width: u16, height: u16) -> Self {
        let original_term_size: (u16, u16) = size().unwrap();
        Self {
            stdout,
            original_term_size,
            width,
            height,
            food: None,
            snek: Snek::new(
                MapPoint::new(width /2, height /2),
                3,
                match rand::thread_rng().gen_range(0,4) {
                    0 => SnekDirection::Up,
                    1 => SnekDirection::Down,
                    2 => SnekDirection::Right,
                    _ => SnekDirection::Left
                },
            ),
            speed: 0,
            score: 0,
            max_score: 20,
            collected_food: Vec::new(),
            flag: vec![
                MapPoint::new(0,0), MapPoint::new(1,17), MapPoint::new(2,2), MapPoint::new(3,26),
                MapPoint::new(4, 13), MapPoint::new(5,14), MapPoint::new(6,27), MapPoint::new(7,18),
                MapPoint::new(8,19), MapPoint::new(9,4), MapPoint::new(10, 15), MapPoint::new(11, 27),
                MapPoint::new(12,14), MapPoint::new(13,13), MapPoint::new(14,27), MapPoint::new(15,18),
                MapPoint::new(16,13), MapPoint::new(17,4), MapPoint::new(18,10), MapPoint::new(19,28)
            ],
        }
    }

    pub fn run(&mut self) {
        self.place_food();
        self.prepare_ui();
        self.render();

        let mut done = false;
        while !done {
            let interval = self.calculate_interval();
            let direction = self.snek.get_direction();
            let now = Instant::now();

            while now.elapsed() < interval {
                if let Some(command) = self.get_command(interval - now.elapsed()) {
                    match command {
                        Command::Quit => {
                            done = true;
                            break;
                        }
                        Command::Turn(towards) => {
                            if direction != towards && direction.opposite() != towards {
                                self.snek.set_direction(towards);
                            }
                        }
                    }
                }
            }

            if self.has_collided_with_wall() || self.has_bitten_itself() {
                done = true;
            } else {
                self.snek.slither();

                if let Some(food_point) = self.food {
                    if self.snek.get_head() == food_point {
                        self.snek.grow();
                        self.score += 1;
                        self.speed += 3;
                        self.collected_food.push(food_point);
                        if self.place_food() {
                            done = true;
                        }
                    }
                }
                self.render();
            }
        }
        self.restore_ui();

        let chars = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "{", "_", "}"];
        println!("\nGame Over! You scored {} points!", self.score);

        if self.collected_food.len() == self.max_score {
            let mut i = 0;
            while i < self.max_score {
                for p in &self.collected_food {
                    if usize::from(p.y) == i {
                        print!("{}", chars[usize::from(p.x)]);
                        i += 1;
                    }
                }
            }
        }
        println!("");
    }

    fn place_food(&mut self) -> bool {
        if usize::from(self.score) == self.max_score || self.flag.is_empty() {
            return true;
        }

        loop {
            let rand_idx = rand::thread_rng().gen_range(0, self.flag.len());
            let p = self.flag[rand_idx];
            let point = MapPoint::new(p.y, p.x);
            if !self.snek.contains_point(&point) {
                self.food = Some(point);
                self.flag.remove(rand_idx);
                break;
            }
        }
        return false;
    }

    fn prepare_ui(&mut self) {
        enable_raw_mode().unwrap();
        self.stdout
            .execute(SetSize(self.width +3, self.height +3)).unwrap()
            .execute(Clear(ClearType::All)).unwrap()
            .execute(Hide).unwrap();
    }

    fn calculate_interval(&self) -> Duration {
        let speed = MAX_SPEED - self.speed;
        Duration::from_millis(
            (MIN_INTERVAL + (((MAX_INTERVAL - MIN_INTERVAL) / MAX_SPEED) * speed)) as u64
        )
    }

    fn get_command(&self, wait_for: Duration) -> Option<Command> {
        let key_event = self.wait_for_key_event(wait_for)?;

        match key_event.code {
            KeyCode::Char('c') | KeyCode::Char('C') =>
                if key_event.modifiers == KeyModifiers::CONTROL {
                    Some(Command::Quit)
                } else {
                    None
                }
            KeyCode::Up     => Some(Command::Turn(SnekDirection::Up)),
            KeyCode::Down   => Some(Command::Turn(SnekDirection::Down)),
            KeyCode::Right  => Some(Command::Turn(SnekDirection::Right)),
            KeyCode::Left   => Some(Command::Turn(SnekDirection::Left)),
            _ => None
        }
    }

    fn wait_for_key_event(&self, wait_for: Duration) -> Option<KeyEvent> {
        if poll(wait_for).ok()? {
            let event = read().ok()?;
            if let Event::Key(key_event) = event {
                return Some(key_event);
            }
        }

        None
    }

    fn has_collided_with_wall(&self) -> bool {
        let head_point = self.snek.get_head();

        match self.snek.get_direction() {
            SnekDirection::Up       => head_point.y == 0,
            SnekDirection::Down     => head_point.y == self.height -1,
            SnekDirection::Right    => head_point.x == self.width -1,
            SnekDirection::Left     => head_point.x == 0
        }
    }

    fn has_bitten_itself(&self) -> bool {
        let next_head_point = self.snek.get_head().transform(self.snek.get_direction(), 1);
        let mut next_body_points = self.snek.get_body().clone();

        // Simulate motion
        next_body_points.remove(next_body_points.len() - 1);
        next_body_points.remove(0);

        next_body_points.contains(&next_head_point)
    }

    fn restore_ui(&mut self) {
        let (cols, rows) = self.original_term_size;
        self.stdout
            .execute(SetSize(cols, rows)).unwrap()
            .execute(Clear(ClearType::All)).unwrap()
            .execute(Show).unwrap()
            .execute(ResetColor).unwrap();
        disable_raw_mode().unwrap();
    }

    fn render(&mut self) {
        self.draw_borders();
        self.draw_background();
        self.draw_food();
        self.draw_snake();
    }

    fn draw_snake(&mut self) {
        let fg = SetForegroundColor(match (self.speed / 3) % 3 {
            0 => Color::Green,
            1 => Color::Cyan,
            _ => Color::Yellow
        });
        self.stdout.execute(fg).unwrap();

        let body_points = self.snek.get_body();
        for (i, body) in body_points.iter().enumerate() {
            let previous = if i == 0 { None } else { body_points.get(i - 1) };
            let next = body_points.get(i + 1);
            let symbol = if let Some(&next) = next {
                if let Some(&previous) = previous {
                    if previous.x == next.x {
                        '║'
                    } else if previous.y == next.y {
                        '═'
                    } else {
                        let d = body.transform(SnekDirection::Down, 1);
                        let r = body.transform(SnekDirection::Right, 1);
                        let u = if body.y == 0 { body.clone() } else { body.transform(SnekDirection::Up, 1) };
                        let l = if body.x == 0 { body.clone() } else { body.transform(SnekDirection::Left, 1) };
                        if (next == d && previous == r) || (previous == d && next == r) {
                            '╔'
                        } else if (next == d && previous == l) || (previous == d && next == l) {
                            '╗'
                        } else if (next == u && previous == r) || (previous == u && next == r) {
                            '╚'
                        } else {
                            '╝'
                        }
                    }
                } else {
                    'O'
                }
            } else if let Some(&previous) = previous {
                if body.y == previous.y {
                    '═'
                } else {
                    '║'
                }
            } else {
                panic!("Invalid snake body point.");
            };

            self.stdout
                .execute(MoveTo(body.x + 1, body.y + 1)).unwrap()
                .execute(Print(symbol)).unwrap();
        }
    }

    fn draw_food(&mut self) {
        self.stdout.execute(SetForegroundColor(Color::White)).unwrap();

        for food in self.food.iter() {
            self.stdout
                .execute(MoveTo(food.x + 1, food.y + 1)).unwrap()
                .execute(Print("•")).unwrap();
        }
    }

    fn draw_background(&mut self) {
        self.stdout.execute(ResetColor).unwrap();

        for y in 1..self.height + 1 {
            for x in 1..self.width + 1 {
                self.stdout
                    .execute(MoveTo(x, y)).unwrap()
                    .execute(Print(" ")).unwrap();
            }
        }
    }

    fn draw_borders(&mut self) {
        self.stdout.execute(SetForegroundColor(Color::DarkGrey)).unwrap();

        for y in 0..self.height + 2 {
            self.stdout
                .execute(MoveTo(0, y)).unwrap()
                .execute(Print("#")).unwrap()
                .execute(MoveTo(self.width + 1, y)).unwrap()
                .execute(Print("#")).unwrap();
        }

        for x in 0..self.width + 2 {
            self.stdout
                .execute(MoveTo(x, 0)).unwrap()
                .execute(Print("#")).unwrap()
                .execute(MoveTo(x, self.height + 1)).unwrap()
                .execute(Print("#")).unwrap();
        }

        self.stdout
            .execute(MoveTo(0, 0)).unwrap()
            .execute(Print("#")).unwrap()
            .execute(MoveTo(self.width + 1, self.height + 1)).unwrap()
            .execute(Print("#")).unwrap()
            .execute(MoveTo(self.width + 1, 0)).unwrap()
            .execute(Print("#")).unwrap()
            .execute(MoveTo(0, self.height + 1)).unwrap()
            .execute(Print("#")).unwrap();

        self.stdout
            .execute(MoveTo(0, self.height + 2)).unwrap()
            .execute(Print(format!("Score: {}\tSpeed: {}", self.score, self.speed))).unwrap();
    }
}


