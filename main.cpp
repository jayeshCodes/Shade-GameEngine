#include <memory>
#include <thread>

#include "main.hpp"
#include "lib/core/timeline.hpp"
#include "lib/ECS/coordinator.hpp"
#include "lib/enum/message_type.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/helpers/colors.hpp"
#include "lib/helpers/constants.hpp"
#include "lib/helpers/ecs_helpers.hpp"
#include "lib/helpers/random.hpp"
#include "lib/model/components.hpp"
#include "lib/systems/camera.cpp"
#include "lib/systems/client.hpp"
#include "lib/systems/collision.hpp"
#include "lib/systems/destroy.hpp"
#include "lib/systems/gravity.cpp"
#include "lib/systems/jump.hpp"
#include "lib/systems/keyboard_movement.cpp"
#include "lib/systems/kinematic.cpp"
#include "lib/systems/move_between_2_point_system.hpp"
#include "lib/systems/receiver.hpp"
#include "lib/systems/render.cpp"
#include <csignal>

class ReceiverSystem;
// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);
Timeline gameTimeline(&anchorTimeline, 1);

void platform_movement(Timeline &timeline, MoveBetween2PointsSystem &moveBetween2PointsSystem) {
    Timeline platformTimeline(&timeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();

    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        moveBetween2PointsSystem.update(dT, platformTimeline);
        auto elapsed_time = platformTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - currentTime); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }
    }

    std::cout << "Kill platform thread" << std::endl;
}

void catch_signals() {
    std::signal(SIGINT, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGTERM, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGSEGV, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
    std::signal(SIGABRT, [](int signal) {
        GameManager::getInstance()->gameRunning = false;
    });
}

void send_delete_signal(zmq::socket_t &client_socket, Entity entity) {
    for (int i = 0; i < 5; i++) {
        std::vector<float> message;
        message.reserve(10);
        message.emplace_back(Message::DELETE);
        for (int i = 0; i < 9; i++) {
            message.emplace_back(0);
        }
        std::string entity_id = gCoordinator.getEntityKey(entity);
        client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        client_socket.send(zmq::buffer(message), zmq::send_flags::none);
    }
}

int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    GameManager::getInstance()->gameRunning = true;
    catch_signals();

    std::string identity = Random::generateRandomID(10);
    std::cout << "Identity: " << identity << std::endl;

    zmq::context_t context(1);
    zmq::socket_t client_socket(context, ZMQ_DEALER);

    client_socket.set(zmq::sockopt::routing_id, identity);
    client_socket.connect("tcp://localhost:5570");

    zmq::pollitem_t items[] = {{client_socket, 0, ZMQ_POLLIN, 0}};

    anchorTimeline.start();
    gameTimeline.start();

    std::vector<std::string> component_names{MAX_COMPONENTS};
    std::vector<std::string> entity_names{MAX_ENTITIES};

    gCoordinator.init();
    gCoordinator.registerComponent<Transform>();
    gCoordinator.registerComponent<Color>();
    gCoordinator.registerComponent<CKinematic>();
    gCoordinator.registerComponent<Camera>();
    gCoordinator.registerComponent<Gravity>();
    gCoordinator.registerComponent<KeyboardMovement>();
    gCoordinator.registerComponent<Receiver>();
    gCoordinator.registerComponent<MovingPlatform>();
    gCoordinator.registerComponent<ClientEntity>();
    gCoordinator.registerComponent<Destroy>();
    gCoordinator.registerComponent<Collision>();
    gCoordinator.registerComponent<Jump>();


    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    auto kinematicSystem = gCoordinator.registerSystem<KinematicSystem>();
    auto gravitySystem = gCoordinator.registerSystem<GravitySystem>();
    auto cameraSystem = gCoordinator.registerSystem<CameraSystem>();
    auto keyboardMovementSystem = gCoordinator.registerSystem<KeyboardMovementSystem>();
    auto moveBetween2PointsSystem = gCoordinator.registerSystem<MoveBetween2PointsSystem>();
    auto destroySystem = gCoordinator.registerSystem<DestroySystem>();
    auto collisionSystem = gCoordinator.registerSystem<CollisionSystem>();
    auto jumpSystem = gCoordinator.registerSystem<JumpSystem>();
    auto clientSystem = gCoordinator.registerSystem<ClientSystem>();
    auto receiverSystem = gCoordinator.registerSystem<ReceiverSystem>();

    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentType<Transform>());
    renderSignature.set(gCoordinator.getComponentType<Color>());
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    Signature kinematicSignature;
    kinematicSignature.set(gCoordinator.getComponentType<Transform>());
    kinematicSignature.set(gCoordinator.getComponentType<CKinematic>());
    gCoordinator.setSystemSignature<KinematicSystem>(kinematicSignature);

    Signature gravitySignature;
    gravitySignature.set(gCoordinator.getComponentType<Transform>());
    gravitySignature.set(gCoordinator.getComponentType<Gravity>());
    gCoordinator.setSystemSignature<GravitySystem>(gravitySignature);

    Signature cameraSignature;
    cameraSignature.set(gCoordinator.getComponentType<Camera>());
    gCoordinator.setSystemSignature<CameraSystem>(cameraSignature);

    Signature keyboardMovementSignature;
    keyboardMovementSignature.set(gCoordinator.getComponentType<Transform>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<CKinematic>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<KeyboardMovement>());
    keyboardMovementSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<KeyboardMovementSystem>(keyboardMovementSignature);

    Signature clientSignature;
    clientSignature.set(gCoordinator.getComponentType<Receiver>());
    gCoordinator.setSystemSignature<ReceiverSystem>(clientSignature);

    Signature movingPlatformSignature;
    movingPlatformSignature.set(gCoordinator.getComponentType<Transform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    movingPlatformSignature.set(gCoordinator.getComponentType<CKinematic>());
    movingPlatformSignature.set(gCoordinator.getComponentType<MovingPlatform>());
    gCoordinator.setSystemSignature<MoveBetween2PointsSystem>(movingPlatformSignature);

    Signature clientEntitySignature;
    clientEntitySignature.set(gCoordinator.getComponentType<ClientEntity>());
    clientEntitySignature.set(gCoordinator.getComponentType<Transform>());
    clientEntitySignature.set(gCoordinator.getComponentType<Color>());
    clientEntitySignature.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<ClientSystem>(clientEntitySignature);

    Signature destroySig;
    destroySig.set(gCoordinator.getComponentType<Destroy>());
    gCoordinator.setSystemSignature<DestroySystem>(destroySig);

    Signature collisionSignature;
    collisionSignature.set(gCoordinator.getComponentType<Transform>());
    collisionSignature.set(gCoordinator.getComponentType<Collision>());
    gCoordinator.setSystemSignature<CollisionSystem>(collisionSignature);

    Signature jumpSignature;
    jumpSignature.set(gCoordinator.getComponentType<Transform>());
    jumpSignature.set(gCoordinator.getComponentType<CKinematic>());
    jumpSignature.set(gCoordinator.getComponentType<Jump>());
    gCoordinator.setSystemSignature<JumpSystem>(jumpSignature);


    Entity mainCamera = gCoordinator.createEntity(Random::generateRandomID(12));
    gCoordinator.addComponent(mainCamera, Camera{
                                  SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 1.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT
                              });
    // temporary values for viewport width and height

    auto mainChar = gCoordinator.createEntity(Random::generateRandomID(12));
    gCoordinator.addComponent(mainChar, Transform{0, SCREEN_HEIGHT - 32, 32, 32, 0});
    gCoordinator.addComponent(mainChar, Color{shade_color::Blue});
    gCoordinator.addComponent(mainChar, CKinematic{});
    gCoordinator.addComponent(mainChar, KeyboardMovement{150.f});
    gCoordinator.addComponent(mainChar, ClientEntity{});
    gCoordinator.addComponent(mainChar, Destroy{});
    gCoordinator.addComponent(mainChar, Jump{100.f, 1.f, false, 0.0f, true, 60.f});
    gCoordinator.addComponent(mainChar, Gravity{0, 100});

    auto clientEntity = gCoordinator.createEntity("CLIENT");
    gCoordinator.addComponent(clientEntity, Receiver{7000, 7001});

    auto last_time = gameTimeline.getElapsedTime();

    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect("tcp://localhost:" + std::to_string(SERVERPORT));
    socket.set(zmq::sockopt::subscribe, "");

    zmq::socket_t pub_socket(context, ZMQ_PUB);

    zmq::socket_t connect_socket(context, ZMQ_REQ);
    connect_socket.connect("tcp://localhost:" + std::to_string(engine_constants::SERVER_CONNECT_PORT));


    std::thread delete_thread([&destroySystem]() {
        while (GameManager::getInstance()->gameRunning) {
        }
    });

    std::thread t1([receiverSystem, &context, &identity]() {
        zmq::socket_t socket(context, ZMQ_DEALER);
        std::string id = identity + "R";
        socket.set(zmq::sockopt::routing_id, id);
        socket.connect("tcp://localhost:5570");
        while (GameManager::getInstance()->gameRunning) {
            receiverSystem->update(socket);
        }
    });

    std::thread t2([&client_socket, &clientSystem] {
        while (GameManager::getInstance()->gameRunning) {
            clientSystem->update(client_socket);
        }
    });

    while (GameManager::getInstance()->gameRunning) {
        doInput();
        prepareScene();

        auto current_time = gameTimeline.getElapsedTime();
        auto dt = (current_time - last_time) / 1000.f; // Ensure this is in seconds

        last_time = current_time;

        dt = std::max(dt, engine_constants::FRAME_RATE); // Cap the maximum dt to 60fps

        kinematicSystem->update(dt);
        cameraSystem->update(dt);
        // collisionSystem->update(dt);
        jumpSystem->update(dt);
        gravitySystem->update(dt);
        keyboardMovementSystem->update();
        destroySystem->update();

        auto main_camera = cameraSystem->getMainCamera();
        auto transform = gCoordinator.getComponent<Transform>(mainChar);
        renderSystem->update(*main_camera, transform.x, transform.y);

        auto elapsed_time = gameTimeline.getElapsedTime();
        auto time_to_sleep = (1.0f / 60.0f) - (elapsed_time - current_time); // Ensure float division
        if (time_to_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_sleep * 1000)));
        }

        presentScene();
    }

    /**
     * This is the cleanup code. The order is very important here since otherwise the program will crash.
     */
    send_delete_signal(client_socket, mainChar);
    delete_thread.join();
    t1.join();
    t2.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
