package server;

import javax.print.DocFlavor;
import java.util.Random;

public class User {
    private String login;
    private int id;
    private int size;

    public User (String login) {
        this.login = login;
        Random random = new Random();
        this.id = random.nextInt(999999);
        this.size = 0;
    }

    public User (String[] info) {
        this.login = info[0];
        this.id = Integer.parseInt(info[1]);
        this.size = Integer.parseInt(info[2]);
    }

    public String getInfo() {
        return login + " " +
                id + " " +
                size;
    }

    public String getLogin() {
        return login;
    }

    public int getId() {
        return id;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }
}
