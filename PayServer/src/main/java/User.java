import javax.print.DocFlavor;
import java.util.Random;

public class User {
    private String login;
    private String password;
    private int id;
    private int size;

    public User (String login, String password) {
        this.login = login;
        this.password = password;
        Random random = new Random();
        this.id = random.nextInt(999999);
        this.size = 0;
    }

    public User (String[] info) {
        this.login = info[0];
        this.password =info[1];
        this.id = Integer.parseInt(info[2]);
        this.size = Integer.parseInt(info[3]);
    }

    public String getInfo() {
        return login + " " +
                password + " " +
                id + " " +
                size;
    }

    public String getLogin() {
        return login;
    }

    public String getPassword() {
        return password;
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
