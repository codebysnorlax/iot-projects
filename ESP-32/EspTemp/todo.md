.counter {
  display: flex;
  font-size: 5rem; /* Adjust size */
  color: #fff;     /* Digit color */
}

.digit {
  position: relative;
  width: 1em;      /* Width of one digit */
  height: 1.2em;   /* Height of one digit */
  overflow: hidden;
  margin: 0 2px;   /* Spacing between digits */
}

.digit .numbers {
  display: flex;
  flex-direction: column;
  transition: transform 0.5s cubic-bezier(0.68, -0.55, 0.27, 1.55);
}

.digit.drop .numbers {
  transition: transform 0.3s ease-in; /* Faster drop animation */
}

.digit .numbers div {
  height: 1.2em;
  line-height: 1.2em;
  text-align: center;
}
