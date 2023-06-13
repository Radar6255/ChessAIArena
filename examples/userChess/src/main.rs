use std::env;
use tungstenite::Message;
// use std::net::TcpListener;

fn main() {
    println!("Hello, world!");

    let args: Vec<String> = env::args().collect();
    dbg!(&args);

    println!("Connecting to {}", args[1]);

    // let server = TcpListener::bind(&args[1]).unwrap();
    let mut server = tungstenite::client::connect(&args[1]).unwrap();
    println!("Connected!");

    let start_msg: Message = server.0.read_message().unwrap();
    let start_arr = start_msg.to_text().unwrap().split(":");
    let mut _game_id: i16;
    let mut is_white: bool = true;

    let mut i = 0;
    for part in start_arr {
        if i == 0 {
            _game_id = part.parse::<i16>().unwrap();
        } else {
            is_white = if part == "white" { true } else { false };
        }
        i += 1;
    }

    if !is_white {
        let first_move: Message = server.0.read_message().unwrap();
    }

    println!("{}", start_msg);
}
