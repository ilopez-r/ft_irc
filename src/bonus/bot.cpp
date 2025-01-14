#include "../../include/Server.hpp"

void sendHelp(Client& sender)
{
	std::string helpMsg = 
		"~ [BOT] Available commands:\n"
		"   * help: Show this help message\n"
		"   * joke: Tell you a random joke\n"
		"   * play: Play rock, paper, scissors\n";
	sender.messageToMyself(helpMsg);
}

void tellJoke(Client& sender)
{
	std::string jokes[] =
	{
		"¿Por qué los pájaros no usan Facebook? Porque ya tienen Twitter. 🐦",
		"¿Qué hace una abeja en el gimnasio? ¡Zum-ba! 🐝",
		"¿Qué hace Lirola cuando tiene que hacer un proyecto? ¡PUM! Me desaparesco. 💨",
		"¿Cómo se dice pañuelo en japonés? Saka-moko. 😷",
		"¿Qué le dice una iguana a su hermana gemela? ¡Iguanita! 🦎",
		"¿Por qué el tomate se puso rojo? Porque vio a la ensalada desnuda. 🍅",
		"¿Qué hace un pez? ¡Nada! 🐟",
		"¿Por qué las plantas odian las matemáticas? Porque les dan raíces cuadradas. 🌿",
		"¿Cómo se despiden los químicos? Ácido un placer. 🧪",
		"¿Qué hace Nestor cuando viene un puma? Fuma. 🚬",
	};
	int randomIndex = rand() % 10;
	sender.messageToMyself("~ [BOT] " + jokes[randomIndex] + "\n");
}

void playGame(Client& sender)
{
	std::string options[] = {"Rock 🪨", "Paper 📄", "Scissors ✂️ "};
	int randomIndex = rand() % 3;
	sender.messageToMyself("~ [BOT] Let's play Rock, Paper, Scissors!\n");
	sender.messageToMyself("~ [BOT] I choose: " + options[randomIndex] + "!\n");
}

void Server::handleBotCommand(Client& sender, const std::string& command)
{

	if (command == "help" || command == "HELP")
		sendHelp(sender);
	else if (command == "joke" || command == "JOKE")
		tellJoke(sender);
	else if (command == "play" || command == "PLAY")
		playGame(sender);
	else
		sender.messageToMyself("~ [BOT] Unknown command. Try: help, joke, or play.\n");
}
