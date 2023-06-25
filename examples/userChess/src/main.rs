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
    println!("{}", start_msg.to_text().unwrap());
    let start_arr = start_msg.to_text().unwrap().split(":");
    let mut _game_id: i16;
    let mut is_white: bool = true;

    let mut i = 0;
    for part in start_arr {
        if i == 0 {
            _game_id = part.parse::<i16>().unwrap();
        } else {
            // println!("{}", part);
            is_white = if part.trim() == "white" { true } else { false };
        }
        i += 1;
    }

    if !is_white {
        let first_move: Message = server.0.read_message().unwrap();
        println!("{}", first_move.to_text().unwrap());
    }

    loop {
        // Here is where we need to let the user input moves to send to the server
        let mut line: String = String::new();
        std::io::stdin().read_line(&mut line).unwrap();

        // let mut input: Message = line;
        let _res = server.0.write_message(tungstenite::Message::Text(line));

        let response: Message = server.0.read_message().unwrap();
        if response.to_text().unwrap() == "true" {
            let oppenent_move: Message = server.0.read_message().unwrap();
            println!("{}", oppenent_move.to_text().unwrap());
        }
    }
}
