// Represents a single chat message between the user and the assistant
class Chat {
  final String role;    // Role of the participant (e.g., user or assistant)
  final String txt;
  final String time;       // Timestamp of the message

  Chat({required this.role, required this.txt, required this.time});

  // Serializes the Chat object to JSON
  Map<String, dynamic> toJson() => {
    'role': role,
    'txt': txt,
    'time': time,
  };

  // Deserializes JSON to create a Chat object
  factory Chat.fromJson(Map<String, dynamic> json) {
    return Chat(
      role: json['role'],
      txt: json['txt'],
      time: json['time'],
    );
  }
}

/*
  Represents chat data including the working state and a history of chats
 */
class ChatSession {
  static final ChatSession _instance = ChatSession._internal();

  String workingState;
  LimitedQueue<Chat> chatHistory;

  ChatSession._internal({
    this.workingState = "",
    int maxChatHistoryLength = 40,
  }) : chatHistory = LimitedQueue<Chat>(maxChatHistoryLength);

  factory ChatSession() {
    return _instance;
  }

  // Serializes the ChatSession object to JSON
  Map<String, dynamic> toJson() => {
    'working_state': workingState,
    'chat_history': chatHistory.items.map((chat) => chat.toJson()).toList()
  };

  // Deserializes JSON to create a ChatSession object
  factory ChatSession.fromJson(Map<String, dynamic> json) {
    var chatHistory = (json['chat_history'] as List)
        .map((i) => Chat.fromJson(i))
        .toList();

    var chatSession = ChatSession();
    chatSession.workingState = json['working_state'];
    chatHistory.forEach(chatSession.chatHistory.add);
    return chatSession;
  }
}

// A limited queue that maintains a maximum length for its items
class LimitedQueue<T> {
  final int maxLength;
  final List<T> _queue = [];

  LimitedQueue(this.maxLength);

  // Adds an item to the queue, removing the oldest item if max length is exceeded
  void add(T item) {
    if (_queue.length >= maxLength) {
      _queue.removeRange(0, 2); // Remove the oldest items
    }
    _queue.add(item);
  }

  // Adds multiple items while respecting the maximum length
  void addAll(Iterable<T> items) {
    for (var item in items) {
      add(item); // Use add method to handle length restriction
    }
  }

  // Returns an unmodifiable list of current items in the queue
  List<T> get items => List.unmodifiable(_queue);

  // Clears the queue
  void clear() {
    _queue.clear();
  }

  // Removes items that match a given condition
  void removeWhere(bool Function(T) test) {
    _queue.removeWhere(test);
  }
}
