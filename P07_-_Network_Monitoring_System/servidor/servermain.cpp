#include "application.hpp"

/*
Servidor:
- Listar quem tá conectado
-- Estatísticas gerais de cada conexão
-- Mostrar processos de cada conectado
--- Matar processo tal
--- Matar e colocar na blacklist
- Lista de quem já se conectou ao menos uma vez
-- Estatísticas gerais de quem já se conectou
- Ver blacklist
-- Deletar/Adicionar na blacklist

Cliente (interface visual mínima):
- Monta a própria lista de processos e envia pra o servidor
- Não deixa processos na blacklist viverem
- Escuta os comandos do servidor

Primeiro opcional:
Alguma coisa com JSON e blablabla
Depois:
Profiling e blablabla
*/

int main(int argc, char const *argv[]) {
	Application app;
	app.launch();

	return 0;
}