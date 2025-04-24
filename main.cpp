#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <sstream>

using namespace sf;

int main()
{
    RenderWindow window(VideoMode(800, 600), "Volleyball Game");
    window.setFramerateLimit(60);

    // Load textures
    Texture tBackground, tBall, tPlayer;
    if (!tBackground.loadFromFile("images/background.png") ||
        !tBall.loadFromFile("images/ball.png") ||
        !tPlayer.loadFromFile("images/blobby.png")) {
        std::cerr << "Failed to load textures!" << std::endl;
        return -1;
    }

    Sprite sBackground(tBackground);
    Sprite sBall(tBall);
    Sprite sPlayer1(tPlayer);
    Sprite sPlayer2(tPlayer);
    sPlayer2.setColor(Color::Red);

    sPlayer1.setPosition(100, 500);
    sPlayer2.setPosition(650, 500);
    sBall.setPosition(400, 300);

    RectangleShape net(Vector2f(10, 200));
    net.setFillColor(Color::White);
    net.setPosition(395, 400);

    float speed = 5.0f;
    float gravity = 0.45f;
    float p1Y = 500, p2Y = 500;
    float p1DY = 0, p2DY = 0;
    bool p1Jumping = false, p2Jumping = false;
    float ballDX = 6.0f;
    float ballDY = 0;

    int score1 = 0, score2 = 0;
    bool reset = false;
    bool gameOver = false;
    Clock resetClock, gameOverClock, gameTimer;
    int countdown = 3;
    std::string loserText = "";

    Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // Player names input
    std::string player1Name = "", player2Name = "";
    bool enteringNames = true;
    bool enteringP1 = true;

    Text inputPrompt("", font, 30);
    Text inputText("", font, 30);
    inputPrompt.setFillColor(Color::White);
    inputText.setFillColor(Color::Cyan);

    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (enteringNames && e.type == Event::TextEntered) {
                if (e.text.unicode == 8) { // Backspace
                    if (enteringP1 && !player1Name.empty()) player1Name.pop_back();
                    else if (!enteringP1 && !player2Name.empty()) player2Name.pop_back();
                }
                else if (e.text.unicode == '\r') { // Enter
                    if (enteringP1) enteringP1 = false;
                    else enteringNames = false;
                }
                else if (e.text.unicode < 128 && e.text.unicode != '\r') {
                    if (enteringP1) player1Name += static_cast<char>(e.text.unicode);
                    else player2Name += static_cast<char>(e.text.unicode);
                }
            }
        }

        if (enteringNames) {
            window.clear();
            inputPrompt.setString(enteringP1 ? "Enter Player 1 Name:" : "Enter Player 2 Name:");
            inputPrompt.setPosition(200, 200);
            inputText.setString(enteringP1 ? player1Name : player2Name);
            inputText.setPosition(200, 250);
            window.draw(inputPrompt);
            window.draw(inputText);
            window.display();
            continue;
        }

        // Start timer
        float elapsedTime = gameTimer.getElapsedTime().asSeconds();
        if (elapsedTime >= 40.0f) {
            gameOver = true;
            if (score1 > score2) loserText = player1Name + " Wins!";
            else if (score2 > score1) loserText = player2Name + " Wins!";
            else loserText = "It's a Tie!";
        }

        if (gameOver) {
            window.clear(Color::Black);
            Text overText(loserText + "\nPress Esc to Exit", font, 30);
            overText.setFillColor(Color::White);
            overText.setPosition(180, 260);
            window.draw(overText);
            window.display();

            if (Keyboard::isKeyPressed(Keyboard::Escape))
                window.close();
            continue;
        }

        if (reset) {
            if (resetClock.getElapsedTime().asSeconds() >= 0.5f) {
                countdown--;
                resetClock.restart();
            }

            if (countdown <= 0) {
                sPlayer1.setPosition(100, 500);
                sPlayer2.setPosition(650, 500);
                sBall.setPosition(400, 300);
                ballDX = 6.0f;
                ballDY = 0;
                countdown = 3;
                reset = false;
            }

            window.clear(Color::Black);
            std::stringstream ss;
            ss << ((countdown > 0) ? std::to_string(countdown) : "Go!");
            Text countdownText(ss.str(), font, 50);
            countdownText.setFillColor(Color::White);
            countdownText.setPosition(365, 260);
            window.draw(countdownText);
            window.display();
            continue;
        }

        // Controls
        if (Keyboard::isKeyPressed(Keyboard::Left) && sPlayer1.getPosition().x > 0)
            sPlayer1.move(-speed, 0);
        if (Keyboard::isKeyPressed(Keyboard::Right) && sPlayer1.getPosition().x + sPlayer1.getGlobalBounds().width < 395)
            sPlayer1.move(speed, 0);
        if (Keyboard::isKeyPressed(Keyboard::Up) && !p1Jumping) {
            p1Jumping = true;
            p1DY = -16.0f;
        }

        // AI Player 2
        Vector2f ballPos = sBall.getPosition();
        float p2X = sPlayer2.getPosition().x;

        if (ballPos.x > 400) {
            if (ballPos.x < p2X)
                sPlayer2.move(-speed, 0);
            else if (ballPos.x > p2X + sPlayer2.getGlobalBounds().width)
                sPlayer2.move(speed, 0);

            if (!p2Jumping && ballPos.y < sPlayer2.getPosition().y - 60) {
                p2Jumping = true;
                p2DY = -16.0f;
            }
        }

        // Keep players in bounds
        if (sPlayer1.getPosition().x < 0)
            sPlayer1.setPosition(0, sPlayer1.getPosition().y);
        if (sPlayer1.getPosition().x + sPlayer1.getGlobalBounds().width > 395)
            sPlayer1.setPosition(395 - sPlayer1.getGlobalBounds().width, sPlayer1.getPosition().y);
        if (sPlayer2.getPosition().x < 405)
            sPlayer2.setPosition(405, sPlayer2.getPosition().y);
        if (sPlayer2.getPosition().x + sPlayer2.getGlobalBounds().width > 800)
            sPlayer2.setPosition(800 - sPlayer2.getGlobalBounds().width, sPlayer2.getPosition().y);

        // Jump physics
        if (p1Jumping) {
            p1DY += gravity;
            p1Y += p1DY;
            if (p1Y >= 500) {
                p1Y = 500;
                p1DY = 0;
                p1Jumping = false;
            }
            sPlayer1.setPosition(sPlayer1.getPosition().x, p1Y);
        }

        if (p2Jumping) {
            p2DY += gravity;
            p2Y += p2DY;
            if (p2Y >= 500) {
                p2Y = 500;
                p2DY = 0;
                p2Jumping = false;
            }
            sPlayer2.setPosition(sPlayer2.getPosition().x, p2Y);
        }

        // Ball physics
        ballDY += gravity;
        ballPos.x += ballDX;
        ballPos.y += ballDY;

        if (ballPos.y < 0) {
            ballPos.y = 0;
            ballDY = -ballDY * 0.8f;
        }
        if (ballPos.x < 0 || ballPos.x > 770)
            ballDX = -ballDX;

        if (sBall.getGlobalBounds().intersects(sPlayer1.getGlobalBounds())) {
            ballDY = -20.0f;
            ballDX = (ballPos.x < sPlayer1.getPosition().x) ? -6.0f : 6.0f;
        }

        if (sBall.getGlobalBounds().intersects(sPlayer2.getGlobalBounds())) {
            ballDY = -20.0f;
            ballDX = (ballPos.x < sPlayer2.getPosition().x) ? -6.0f : 6.0f;
        }

        if (ballPos.y > 550) {
            if (ballPos.x < 400) score2++;
            else score1++;
            reset = true;
            resetClock.restart();
        }

        sBall.setPosition(ballPos);

        // Draw
        window.clear();
        window.draw(sBackground);
        window.draw(net);
        window.draw(sBall);
        window.draw(sPlayer1);
        window.draw(sPlayer2);

        Text name1(player1Name, font, 15);
        name1.setPosition(sPlayer1.getPosition().x, sPlayer1.getPosition().y - 25);
        window.draw(name1);

        Text name2(player2Name, font, 15);
        name2.setPosition(sPlayer2.getPosition().x, sPlayer2.getPosition().y - 25);
        window.draw(name2);

        Text scoreText(player1Name + ": " + std::to_string(score1) + " | " + player2Name + ": " + std::to_string(score2), font, 20);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(260, 10);
        window.draw(scoreText);

        Text timerText("Time: " + std::to_string(static_cast<int>(40 - elapsedTime)), font, 20);
        timerText.setFillColor(Color::White);
        timerText.setPosition(10, 10);
        window.draw(timerText);

        window.display();
    }

    return 0;
}

// g++ main.cpp -IC:\Users\kaurg\Downloads\SFML-2.6.2\include -LC:\Users\kaurg\Downloads\SFML-2.6.2\lib -lsfml-graphics -lsfml-window -lsfml-system -o VolleyballGame.exe
// .\VolleyballGame