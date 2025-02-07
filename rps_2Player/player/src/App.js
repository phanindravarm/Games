import { useState } from "react";
function App() {
  const [isClicked, setIsClicked] = useState(false);
  const [result, setResult] = useState("");

  const getMove = async (move) => {
    try {
      console.log("entered");
      const response = await fetch(`http://localhost:8080/${move}`);
      console.log("fetched");
      const data = await response.text();
      console.log(`data : ${data}`);
      if (data.includes("Wins") || data.includes("Tie")) {
        setResult(data);
        setIsClicked(false);
      } else {
        console.log("entered else");
        getMove(move);
      }
    } catch (error) {
      console.error("Error:", error);
      setResult("hey");
      setIsClicked(false);
    }
  };

  const handleClick = (move) => {
    setResult("Waiting for player 2 to pick his move");
    getMove(move);
    setIsClicked(true);
  };

  return (
    <div className="App">
      <button
        onClick={() => {
          handleClick("Rock");
        }}
        disabled={isClicked}
      >
        Rock
      </button>
      <button onClick={() => handleClick("Paper")} disabled={isClicked}>
        Paper
      </button>
      <button onClick={() => handleClick("Scissors")} disabled={isClicked}>
        Scissors
      </button>
      <p>{result}</p>
    </div>
  );
}

export default App;
