// Need to create the chess board first really
let board = document.getElementById("board");

for (let x = 0; x < 8; x++) {
    const newRow = document.createElement("tr");
    for (let y = 0; y < 8; y++) {
        const tableData = document.createElement("td");
        tableData.setAttribute("style", "width: 60px; height: 60px;");
        tableData.id = `${x}${y}`;

        newRow.appendChild(tableData);
    }

    board.appendChild(newRow);
}

// Now we need to setup the inital board
let back = ["r", "n", "b", "q", "k", "b", "n", "r"];

for (let x = 0; x < 8; x++) {
    const blackEl = document.getElementById(`7${x}`);
    const bpiece = document.createElement("img");
    bpiece.setAttribute("src", `resources/b${back[x]}.png`);
    blackEl.appendChild(bpiece);


    const whiteEl = document.getElementById(`0${x}`);
    const fpiece = document.createElement("img");
    fpiece.setAttribute("src", `resources/w${back[x]}.png`);
    whiteEl.appendChild(fpiece);

    const blackPawnEl = document.getElementById(`6${x}`);
    const blackPawn = document.createElement("img");
    blackPawn.setAttribute("src", `resources/bp.png`);
    blackPawnEl.appendChild(blackPawn);

    const whitePawnEl = document.getElementById(`1${x}`);
    const whitePawn = document.createElement("img");
    whitePawn.setAttribute("src", `resources/wp.png`);
    whitePawnEl.appendChild(blackPawn);
}

function boardPosToCoords(boardPos){
    return [boardPos.charCodeAt(0) - 97, parseInt(boardPos[1], 10) - 1,
        boardPos.charCodeAt(2) - 97, parseInt(boardPos[3], 10) - 1];
}

function performMove(moveCoords){
    const src = document.getElementById(`${moveCoords[0]}${moveCoords[1]}`);
    const dst = document.getElementById(`${moveCoords[2]}${moveCoords[3]}`);

    let movingPiece = src.children[0];
    dst.children[0].remove();
    dst.appendChild(movingPiece);

    src.removeChild(movingPiece);
}

// Connecting to game 0 on the server
let webSocket = new WebSocket("ws://localhost:8000/ws/client");

webSocket.onopen = (event) => {
    webSocket.send("0");
}

webSocket.onmessage = (event) => {
    console.log(event.data);

    let coords = boardPosToCoords(event.data);
    performMove(coords);
}
