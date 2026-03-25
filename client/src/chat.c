#include "../lib/all.h"

static void free_chat_message(void* data) {
	free(data);
}

int chat_init(Chat* chat, int max_messages) {
	if (!chat || max_messages <= 0) return EXIT_FAILURE;

	chat->messages = list_create();
	if (!chat->messages) return EXIT_FAILURE;

	chat->max_messages = max_messages;
	return EXIT_SUCCESS;
}

int chat_push(Chat* chat, const char* message) {
	if (!chat || !chat->messages || !message) return EXIT_FAILURE;

	char* copy = strdup(message);
	if (!copy) return EXIT_FAILURE;

	if (list_add(chat->messages, copy) != EXIT_SUCCESS) {
		free(copy);
		return EXIT_FAILURE;
	}

	while (chat->messages->size > chat->max_messages) {
		ListNode* oldest = chat->messages->head;
		if (!oldest) break;

		chat->messages->head = oldest->next;
		chat->messages->size--;

		free_chat_message(oldest->data);
		free(oldest);
	}

	return EXIT_SUCCESS;
}

const char* chat_get(Chat* chat, int index) {
	if (!chat || !chat->messages) return NULL;
	return (const char*)list_get(chat->messages, index);
}

int chat_size(Chat* chat) {
	if (!chat || !chat->messages) return 0;
	return list_size(chat->messages);
}

void chat_clear(Chat* chat) {
	if (!chat || !chat->messages) return;

	list_destroy(chat->messages, free_chat_message);
	chat->messages = NULL;
	chat->max_messages = 0;
}
