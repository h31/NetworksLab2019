package server;

import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;


public class ServerSession {
    private static final String PATH_TO_CLIENTS_INFO = "src/main/resources/ClientsInfo";
    private static final String ERR_SIGN = "Wrong login";
    private static final String WRONG_FORMAT = "Wrong format";
    private Socket socket;
    private Server server;
    private BufferedReader in;
    private BufferedWriter out;
    private User user;
    private boolean connect;
    private Condition condition;
    private ServerSession from;
    private int size;

    public ServerSession(Socket socket, Server server) throws IOException {
        this.socket = socket;
        this.server = server;
        connect = true;
        condition = Condition.NORMAL;
        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
        run();
    }


    public void run() {
        new Thread(() -> {
            while (connect) {
                try {
                    String msg = in.readLine();
                    if (msg == null) {
                        disconnect();
                        return;
                    }
                    if (user != null) System.out.println("from " + user.getLogin() + ": " + msg);
                    switch (condition) {
                        case NORMAL:
                            try {
                                normalHandler(msg);
                            } catch (NumberFormatException e) {
                                send(WRONG_FORMAT);
                            }
                            break;
                        case ACCEPTING:
                            acceptingHandler(msg);
                            break;
                        case WAITING_FOR_ACCEPT:
                            send("Can't do this operation\nWaiting for accept...");
                    }
                } catch (IOException e) {
                    this.downService();
                }
            }

        }).start();
    }

    private void send(String msg) {
        try {
            System.out.println("to " + user.getLogin() + ": " + msg);
            out.write(msg + "\n");
            out.flush();
        } catch (IOException ignored) {
        }
    }

    private void normalHandler(String msg) throws IOException, NumberFormatException {
        String[] strings = msg.split("\\s+");
        switch (strings[0]) {
            case "signin":
                signin(strings[1]);
                break;
            case "signup":
                signup(strings[1]);
                break;
            case "getinfo":
                if (strings.length == 1) getInfo();
                else send(WRONG_FORMAT);
                break;
            case "list":
                if (strings.length == 1) getList();
                else send(WRONG_FORMAT);
                break;
            case "transaction":
                if (strings.length != 3) {
                    send(WRONG_FORMAT);
                } else transaction(Integer.parseInt(strings[1]), Integer.parseInt(strings[2]));
                break;
            case "put":
                if (strings.length != 2) {
                    send(WRONG_FORMAT);
                } else {
                    user.setSize(user.getSize() + Integer.parseInt(strings[1]));
                    Utils.updateUserSizeInfo(user, PATH_TO_CLIENTS_INFO);
                    send("Complete");
                }
                break;
            case "take":
                if (strings.length != 2) {
                    send(WRONG_FORMAT);
                } else {
                    if (user.getSize() < Integer.parseInt(strings[1])) {
                        send("Not enough money");
                    } else {
                        user.setSize(user.getSize() - Integer.parseInt(strings[1]));
                        Utils.updateUserSizeInfo(user, PATH_TO_CLIENTS_INFO);
                        send("Complete");
                    }
                }
                break;
            case "help":
                send("getinfo - get information about your account\n" +
                        "list - get list of registered clients\n" +
                        "put <n> - put n money\n" +
                        "take <n> - take n money\n" +
                        "transaction <r> <n> - make a transaction to r(purse number of recipient) at n(money)");
                break;
            case "/exit":
                send("exit");
                disconnect();
                break;
            default:
                send("Unknown command");
        }
    }

    private void acceptingHandler(String msg) throws IOException {
        switch (msg) {
            case "yes":
                user.setSize(user.getSize() + size);
                Utils.updateUserSizeInfo(user, PATH_TO_CLIENTS_INFO);
                from.getUser().setSize(from.getUser().getSize() - size);
                Utils.updateUserSizeInfo(from.getUser(), PATH_TO_CLIENTS_INFO);
                from.send("Transaction completed");
                from.setCondition(Condition.NORMAL);
                condition = Condition.NORMAL;
                break;
            case "no":
                from.send("Transaction denied");
                from.setCondition(Condition.NORMAL);
                condition = Condition.NORMAL;
                break;
            default:
                send("Unknown command\nAccept? (yes/no)");
        }

    }

    private void transaction(int id, int size) throws IOException {
        if (user.getSize() < size) {
            send("Sorry, not enough money");
            return;
        }
        for (ServerSession s : server.getClientList()) {
            if (s.getUser().getId() == id) {
                if (s.getCondition() == Condition.NORMAL) {
                    condition = Condition.WAITING_FOR_ACCEPT;
                    s.acceptTransaction(this, size);
                    return;
                } else {
                    send(s.getUser().getLogin() + " is busy with another transaction");
                }
            }
        }
        send("Wrong purse number or client is offline");
    }

    public void acceptTransaction(ServerSession from, int size) {
        if (condition == Condition.NORMAL) {
            send("Input transaction from: " + from.getUser().getLogin() +
                    "[" + from.getUser().getId() +
                    "] " + size +
                    "$\nAccept? (yes/no)");
            condition = Condition.ACCEPTING;
            this.from = from;
            this.size = size;
        } else from.send(user.getLogin() + " is busy with another transaction");
    }

    private void getList() throws IOException {
        StringBuilder stringBuilder = new StringBuilder();
        for (String str : Files.readAllLines(Paths.get(PATH_TO_CLIENTS_INFO))) {
            String[] line = str.split("\\s+");
            stringBuilder.append("Login: ").append(line[0]).append("\t\t");
            stringBuilder.append("Purse number: ").append(line[1]).append("\n");
        }
        stringBuilder.setLength(stringBuilder.length() - 1);
        send(stringBuilder.toString());
    }

    private void getInfo() {
        String str = "Login: " + user.getLogin() + "\n" +
                "Purse number: " + user.getId() + "\n" +
                "Purse size: " + user.getSize();
        send(str);
    }

    private void signin(String login) throws IOException {
        user = getUserInfo(login);
        if (user == null) {
            send(ERR_SIGN);
            return;
        }
        System.out.println(user.getLogin() + " is connected");
        send("Enter complete");
    }

    private void signup(String login) throws IOException {
        user = new User(login);
        for (String str : Files.readAllLines(Paths.get(PATH_TO_CLIENTS_INFO))) {
            String[] line = str.split("\\s+");
            if (line[0].equals(login)) {
                send("This login is already used");
                return;
            }
        }
        Utils.addLineInFile(user.getInfo() + "\n", PATH_TO_CLIENTS_INFO);
        System.out.println(user.getLogin() + " is connected");
        send("Enter complete");
    }

    private void downService() {
        try {
            if (!socket.isClosed()) {
                socket.close();
                in.close();
                out.close();
                for (ServerSession vr : server.getClientList()) {
                    if (vr.equals(this))
                        server.removeClient(this);
                }
            }
        } catch (IOException ignored) {
        }
    }

    private User getUserInfo(String login) throws IOException {
        List<String> lines = Files.readAllLines(Paths.get(PATH_TO_CLIENTS_INFO));
        for (String l : lines) {
            String[] strings = l.split("\\s+");
            if (strings[0].equals(login)) {
                return new User(strings);
            }
        }
        return null;
    }

    private void disconnect() throws IOException {
        socket.close();
        server.removeClient(this);
        connect = false;
    }

    public User getUser() {
        return user;
    }

    public Condition getCondition() {
        return condition;
    }

    public void setCondition(Condition condition) {
        this.condition = condition;
    }
}
