import subprocess
import chess

def mirror_fen(fen):
    board = chess.Board(fen)
    mirrored = board.mirror()
    return mirrored.fen()

def get_eval(fen):
    p = subprocess.Popen(["ADAM.exe"], cwd="c:/Users/sriva/OneDrive/Desktop/Adam",
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    commands = f"position fen {fen}\neval\nquit\n"
    out, err = p.communicate(input=commands, timeout=5)
    for line in out.splitlines():
        if "Eval:" in line:
            # line is: Eval: X cp
            parts = line.split()
            return int(parts[1])
    raise RuntimeError(f"No eval found for {fen}: {out}")

test_fens = [
    # Startpos
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    # 1. e4
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
    # Castled positions
    "r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 7",
    # The game from user! Move 14 before 15. Rxf6:
    # 1. c4 e5 2. Nc3 Nf6 3. Nf3 Nc6 4. e4 Bb4 5. d3 d6 6. Be2 Bg4 7. a3 Ba5 8. Rb1 O-O 9. O-O Bb6 10. Be3 Bxe3 11. fxe3 h6 12. b4 a5 13. b5 Ne7 14. Nh4 Qd7
    "r4rk1/1ppqnpp1/3p1n1p/pP2p3/4P2N/P1NPB3/2P1B1PP/1R1Q1RK1 w - - 1 15",
    # Move 15 after Rxf6 Bxe2 16. Nxe2 gxf6:
    "r4rk1/1ppqnp2/3p1p1p/pP2p3/4P2N/P1NP4/2P1N1PP/1R1Q2K1 w - - 0 17",
    # Pawn endgame
    "8/5k2/8/4p3/4P3/8/5K2/8 w - - 0 1",
    # Rook endgame
    "8/1r3k2/8/4p3/4P3/8/1R3K2/8 w - - 0 1",
    # Passed pawn
    "8/8/4k3/8/4P3/8/8/4K3 w - - 0 1",
    # Isolated pawns
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    # Asymmetric test
    "r1bqk2r/pp1n1ppp/2p1pn2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQkq - 0 7",
    # Queen and king
    "4k3/8/8/8/8/8/4Q3/4K3 w - - 0 1",
    # Knight outpost
    "r1bqk2r/ppp2ppp/2n5/3np3/8/2NP1N2/PPP2PPP/R1BQKB1R w KQkq - 0 7",
]

for i, fen in enumerate(test_fens):
    mfen = mirror_fen(fen)
    e1 = get_eval(fen)
    e2 = get_eval(mfen)
    diff = e1 - e2
    status = "OK" if diff == 0 else f"ASYMMETRIC (diff: {diff})"
    print(f"[{i+1}] {status} | original={e1} cp | mirror={e2} cp")
    if diff != 0:
        print(f"   FEN:  {fen}")
        print(f"   MFEN: {mfen}")
