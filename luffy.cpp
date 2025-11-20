#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cmath>

int main() {
    // Configuración de la ventana
    sf::RenderWindow window(sf::VideoMode(1920, 1080), 
                           "Robot Delivery Simulator - Miami", 
                           sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // =========================================================================
    // CONFIGURACIÓN DEL SISTEMA
    // =========================================================================
    
    // Parámetros de velocidad
    const float GEAR_SPEEDS[] = {0.0f, 2.07f, 2.88f, 4.05f};
    const float REVERSE_SPEED = -1.38f;
    const float ACCELERATION = 0.1f;
    
    // Parámetros de control
    const float ROTATION_SENSITIVITY = 0.02f;
    const float JOYSTICK_DEADZONE = 25.0f;
    const float ROBOT_SCALE = -0.05f;

    // =========================================================================
    // INICIALIZACIÓN DE RECURSOS
    // =========================================================================
    
    // Carga del mapa de fondo
    sf::Texture mapTexture;
    sf::Sprite mapBackground;
    bool mapLoaded = false;
    
    if (!mapTexture.loadFromFile("img/miami_map.png")) {
        std::cout << "Error: No se pudo cargar el mapa img/miami_map.png" << std::endl;
    } else {
        mapBackground.setTexture(mapTexture);
        mapLoaded = true;
        float scaleX = 1920.0f / mapTexture.getSize().x;
        float scaleY = 1080.0f / mapTexture.getSize().y;
        mapBackground.setScale(scaleX, scaleY);
    }

    // Carga del vehículo robot
    sf::Texture robotTexture;
    sf::Sprite robot;
    
    if (!robotTexture.loadFromFile("img/robot.png")) {
        sf::Image tempImage;
        tempImage.create(80, 120, sf::Color(200, 50, 50));
        robotTexture.loadFromImage(tempImage);
    }
    
    robot.setTexture(robotTexture);
    robot.setScale(ROBOT_SCALE, 0.05f);
    robot.setOrigin(robotTexture.getSize().x / 2.0f, robotTexture.getSize().y / 2.0f);
    robot.setPosition(960.0f, 540.0f);

    // =========================================================================
    // VARIABLES DE ESTADO
    // =========================================================================
    float currentSpeed = 0.0f;
    int currentGear = 1;

    bool triangleWasPressed = false;
    bool xWasPressed = false;

    // Cargar fuente
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    }

    // =========================================================================
    // BUCLE PRINCIPAL
    // =========================================================================
    while (window.isOpen()) {
        // Procesamiento de eventos
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
        }

        // Controles
        if (sf::Joystick::isConnected(0)) {
            bool deadmanActive = sf::Joystick::isButtonPressed(0, 4);
            float stickX = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
            
            if (std::abs(stickX) > JOYSTICK_DEADZONE) {
                float rotationDirection = (currentSpeed < -0.1f) ? -1.0f : 1.0f;
                robot.rotate(stickX * ROTATION_SENSITIVITY * rotationDirection);
            }
            
            if (deadmanActive) {
                if (sf::Joystick::isButtonPressed(0, 7)) {
                    currentSpeed = std::min(currentSpeed + ACCELERATION, GEAR_SPEEDS[currentGear]);
                }
                else if (sf::Joystick::isButtonPressed(0, 6)) {
                    currentSpeed = std::max(currentSpeed - ACCELERATION, REVERSE_SPEED);
                }
                else {
                    currentSpeed *= 0.9f;
                    if (std::abs(currentSpeed) < 0.05f) currentSpeed = 0.0f;
                }
            } else {
                currentSpeed *= 0.7f;
                if (std::abs(currentSpeed) < 0.02f) currentSpeed = 0.0f;
            }
            
            bool trianglePressed = sf::Joystick::isButtonPressed(0, 3);
            bool xPressed = sf::Joystick::isButtonPressed(0, 1);
            
            if (trianglePressed && !triangleWasPressed && currentGear < 3) currentGear++;
            if (xPressed && !xWasPressed && currentGear > 1) currentGear--;
            
            triangleWasPressed = trianglePressed;
            xWasPressed = xPressed;
        }

        // Física
        if (std::abs(currentSpeed) > 0.01f) {
            float angleRad = robot.getRotation() * 3.141592f / 180.0f;
            robot.move(std::cos(angleRad) * currentSpeed, std::sin(angleRad) * currentSpeed);
        }

        // Límites
        sf::FloatRect bounds = robot.getGlobalBounds();
        sf::Vector2f position = robot.getPosition();
        float margin = bounds.width / 2.0f;
        position.x = std::max(margin, std::min(1920.0f - margin, position.x));
        position.y = std::max(margin, std::min(1080.0f - margin, position.y));
        robot.setPosition(position);

        // =====================================================================
        // RENDERIZADO
        // =====================================================================
        window.clear(sf::Color::Black);
        
        // 1. Mapa de fondo
        if (mapLoaded) {
            window.draw(mapBackground);
        }
        
        // 2. Robot
        window.draw(robot);
        
        // 3. TACÓMETRO (INFERIOR DERECHA - LIMPIO Y PROFESIONAL)
        // ======================================================
        
        // Fondo semi-transparente
        sf::RectangleShape tacoBackground(sf::Vector2f(200, 80));
        tacoBackground.setFillColor(sf::Color(0, 0, 0, 180));
        tacoBackground.setPosition(1650, 970);  // Esquina inferior derecha
        
        // Texto de velocidad
        sf::Text speedText;
        speedText.setFont(font);
        speedText.setCharacterSize(28);
        speedText.setFillColor(sf::Color::Red);
        
        std::string speedStr = std::to_string(currentSpeed).substr(0, 5) + " m/s";
        speedText.setString(speedStr);
        speedText.setPosition(1660, 975);
        
        // Texto del cambio
        sf::Text gearText;
        gearText.setFont(font);
        gearText.setCharacterSize(36);
        gearText.setFillColor(sf::Color::Red);
        gearText.setStyle(sf::Text::Bold);
        
        std::string gearStr = (currentSpeed < -0.1f) ? "R" : std::to_string(currentGear);
        gearText.setString(gearStr);
        gearText.setPosition(1850, 980);
        
        // Dibujar tacómetro
        window.draw(tacoBackground);
        window.draw(speedText);
        window.draw(gearText);

        window.display();
    }

    return 0;
}