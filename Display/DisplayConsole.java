import java.net.Socket;
import java.nio.CharBuffer;
import java.io.*;
import java.util.Arrays;
import java.util.Scanner;
import java.nio.CharBuffer;
import java.awt.Point;
import java.util.concurrent.*;

public class DisplayConsole {

  private Thread displayThread;
  private volatile boolean running = true;
  private volatile boolean wrongResult = false;
  private int port = 12345;
  private int pingInterval = 30;
  private int vue_height;
  private int vue_width;
  private boolean requestContinuously = false;
  private Socket sock;
  private String addr = "localhost";
  private Display display;
  private ScheduledExecutorService updateService = Executors.newSingleThreadScheduledExecutor();
  private final ScheduledExecutorService pingService = Executors.newSingleThreadScheduledExecutor();

  private void startDisplay() {
    display = new Display(vue_width, vue_height);
    displayThread = new Thread(display);
    displayThread.start();
  }

  private void endDisplay() {
    display.terminate();
  }

  private void addFishesParser(String buffer) {
    String rmaddFish = buffer.substring(8, buffer.length());
    String[] fishes_value = rmaddFish.split(", ");
    String[] nameAndPos = fishes_value[0].split(" at ");
    String name = nameAndPos[0];
    String[] pos = nameAndPos[1].split("x");
    String[] dim = fishes_value[1].split("x");
    int xpos = Integer.parseInt(pos[0]);
    int ypos = Integer.parseInt(pos[1]);
    int xdim = Integer.parseInt(dim[0]);
    int ydim = Integer.parseInt(dim[1]);
    String mobility = fishes_value[2];
    display.vue.addFish(new Fish(name, mobility, xpos, ypos, xdim, ydim));
  }

  private void majFishesParser(String buffer) { // suppression et ajout poisson a discuter
    boolean isStatus = buffer.startsWith("stat");
    String rmlist = buffer.substring(4, buffer.length());
    String[] strings = rmlist.split("]");
    int nbFishes = strings.length;
    if (rmlist.equals("")){
      nbFishes = 0;
    }

    if (isStatus) {
      System.out.println(
          "\t -> OK : Connecté au contrôleur, " + nbFishes + (nbFishes > 1 ? " poissons trouvés" : " poisson trouvé"));
    }

    for (int i = 0; i < nbFishes; i++) {
      String s = strings[i].substring(2, strings[i].length());

      // Remplacer la pos par la taille

      String[] fishes_value = s.split(",");
      String[] nameAndPos = fishes_value[0].split(" at ");
      String name = nameAndPos[0];
      String[] pos = nameAndPos[1].split("x");
      String[] dest = fishes_value[1].split("x");
      int xpos = Integer.parseInt(pos[0]);
      int ypos = Integer.parseInt(pos[1]);
      int destx = Integer.parseInt(dest[0]);
      int desty = Integer.parseInt(dest[1]);
      int timetoarrived = Integer.parseInt(fishes_value[2]);
      if (isStatus) {
        System.out.println("\tFish " + name + " at " + xpos + "x" + ypos + "," + destx + "x" + desty + " "
            + (timetoarrived == 0 ? "notStarted" : "started"));
      } else {
        boolean found = false;
        for (int j = 0; j < display.vue.fishes.size(); j++) {
          if (display.vue.fishes.get(j).getName().compareTo(name) == 0) {
            found = true;
            Point prevDest = display.vue.fishes.get(j).getDestination();
            if ((int) prevDest.getX() != destx || (int) prevDest.getY() != desty) {
              display.vue.fishes.get(j).setPosition(xpos, ypos); // A enlever
              Point desti = new Point(destx, desty); // normalement destination donner par controleur
              display.vue.fishes.get(j).set_move(desti, timetoarrived, display.refreshTime);
              break;
            }
          }
        }
        if (!found) {
          display.vue.addFish(new Fish(name, "RandomWayPoint", xpos, ypos, 60, 60));
          for (int k = 0; k < display.vue.fishes.size(); k++) {
            if (display.vue.fishes.get(k).getName().compareTo(name) == 0) {
              display.vue.fishes.get(k).startFish();
              break;
            }
          }
        }
      }
    }
  }

  private boolean openConnection(String param) throws Exception {

    Socket socket = new Socket(addr, port);

    sock = socket;

    String message = "hello";
    if (!param.equals("")) {
      message = message.concat(" in as ");
      message = message.concat(param);
    }
    try {
      sendPacket(message);
      String res = recvPacket();
      if (res.equals("no greeting")) {
        return false;
      } else {
        String[] words = res.split(" ");
        vue_width = Integer.parseInt(words[2]);
        vue_height = Integer.parseInt(words[3]);
        System.out.println("Bienvenue, vous observez maintenant la vue " + words[1]);
      }

    } catch (Exception e) {
      System.err.println("Error communicating with server");
      return false;
    }
    return true;
  }

  private void endConnection() {
    try {
      String message = "log out";
      sendPacket(message);
      String res = recvPacket();
      if (res.equals("bye")) {
        sock.close();
      }
    } catch (Exception e) {
      System.err.println("Error closing socket");
    }
  }

  private synchronized void sendPacket(String message) throws Exception {

    PrintWriter writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(sock.getOutputStream())), true);

    writer.print(message);
    writer.flush();
  }

  private synchronized String recvPacket() throws Exception {
    // recup messages serveur
    BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));

    char[] buffer = new char[1024];
    int nb = reader.read(buffer);
    buffer = Arrays.copyOf(buffer, nb);
    String str = String.copyValueOf(buffer);
    return str;
  }

  private boolean checkAvailability() {
    try {
      sendPacket("ping 12345");
      String message = recvPacket();
    } catch (Exception e) {
      return false;
    }
    return true;
  }

  private void maintainConnexion() {
    if (!checkAvailability()) {
      running = false;
    }
  }

  private void updatePeriodic(){
    try{
      sendPacket("getFishes");
      String message = recvPacket();
      while (!message.startsWith("list")){
        wrongResult = true;
        sendPacket("getFishes");
        message = recvPacket();
      }
      majFishesParser(message);
    } catch (Exception e){
      e.printStackTrace();
      running = false;
    }
  }

  private void handleCommand(String cmd, String answer) {
    while (answer.startsWith("list")){
      majFishesParser(answer);
      try{
        if (wrongResult){
          sendPacket(cmd);
        }
        answer = recvPacket();
      } catch (Exception e){
        running = false;
      }
    }
    wrongResult = false;

    while ((cmd.startsWith("addFish") || cmd.startsWith("delFish") || cmd.startsWith("startFish") || cmd.equals("status") || cmd.equals("switch")) && answer.startsWith("NOK1")){
      try{
        sendPacket(cmd);
        answer = recvPacket();
      } catch (Exception e){
        running = false;
      }
    }

    if (cmd.startsWith("addFish")) {
      if (answer.startsWith("OK")){
        addFishesParser(cmd);
        System.out.println("\t -> OK");
      } else {
        // Differentes reponses selon l'erreur?
        System.out.println("\t -> NOK");
      }
    } else if (cmd.startsWith("startFish")) {
      if (answer.startsWith("OK")){
      String fname = cmd.substring(10, cmd.length());
      for (int i = 0; i < display.vue.fishes.size(); i++) {
        if (display.vue.fishes.get(i).getName().compareTo(fname) == 0) {
          display.vue.fishes.get(i).startFish();
          break;
        }
      }
      System.out.println("\t -> OK");
    } else {
      System.out.println("\t -> NOK : Poisson inexistant");
    }
    } else if (cmd.startsWith("delFish")) {
      if (answer.startsWith("OK")){
      String fname = cmd.substring(8, cmd.length());
      for (int i = 0; i < display.vue.fishes.size(); i++) {
        if (display.vue.fishes.get(i).getName().compareTo(fname) == 0) {
          display.removeFish(display.vue.fishes.get(i));
          break;
        }
      }
      System.out.println("\t -> OK");
    } else {
      System.out.println("\t -> NOK : Poisson inexistant");
    }
    } else if (cmd.equals("status")) {
      if (answer.startsWith("stat")){
        majFishesParser(answer);
      } else {
        System.out.println("\t -> NOK");
      }
    } else if (cmd.equals("switch")){
      if (requestContinuously){
        requestContinuously = false;
        try {
          sendPacket("stopContinuous");
        String dump = recvPacket();
        } catch (Exception e) {
          running = false;
        }
        updateService = Executors.newSingleThreadScheduledExecutor();
        updateService.scheduleAtFixedRate(new Runnable() {
          @Override
          public void run() {
            updatePeriodic();
          }
        }, 0, 1, TimeUnit.SECONDS);
        System.out.println("\t -> Switched to periodic mode");
      } else {
        try {
          sendPacket("getFishesContinuously");
          String received = recvPacket();
          if (received.startsWith("list")) {
            majFishesParser(received);
          }
          requestContinuously = true;

        } catch (Exception e) {
          running = false;
        }
        updateService.shutdown();
        System.out.println("\t -> Switched to continuous mode");
      }
    } /*else if (cmd.equals("getFishes") && answer.startsWith("list")) {
      if (answer.length() > 4) {
        majFishesParser(answer);
      }
    } else if (cmd.equals("getFishesContinuously") && answer.startsWith("list")) {
      if (answer.length() > 4) {
        majFishesParser(answer);
      }
      requestContinuously = true;
      updateService.shutdown();
    } else if (cmd.equals("stopContinuous") && answer.startsWith("OK")) {
      if (requestContinuously) {
        requestContinuously = false;
        updateService.scheduleAtFixedRate(new Runnable() {
          @Override
          public void run() {
            updatePeriodic();
          }
        }, 0, 1, TimeUnit.SECONDS);
      }
    } */else {
      System.out.println("\t -> NOK : Commande introuvable");
    }
  }

  public static void main(String[] args) {
    BufferedReader scanner = new BufferedReader(new InputStreamReader(System.in));
    DisplayConsole dc = new DisplayConsole();
    boolean res = false;

    System.out.print(
        "Veuillez entrer le nom de la vue à afficher, si vous souhaitez obtenir la premiere vue disponible ne donnez pas de nom : ");
    String view_name = "";
    try {
      view_name = scanner.readLine();
    } catch (IOException e) {
      System.err.println("BufferedReader error");
    }

    try {
      res = dc.openConnection(view_name);
    } catch (Exception e) {
      System.err.println("Error creating socket");
    }

    if (res) {

      dc.startDisplay();
      dc.pingService.scheduleAtFixedRate(new Runnable() {
        @Override
        public void run() {
          dc.maintainConnexion();
        }
      }, 0, dc.pingInterval, TimeUnit.SECONDS);

      try{
        Thread.sleep(500);
      } catch (InterruptedException e){
        System.err.println("Erreur sleep");
      }
      dc.updateService.scheduleAtFixedRate(new Runnable() {
        @Override
        public void run() {
          dc.updatePeriodic();
        }
      }, 0, 1, TimeUnit.SECONDS);

      boolean serverStopped = false;
      BufferedReader socketStream;
      try {
        socketStream = new BufferedReader(new InputStreamReader(dc.sock.getInputStream()));

        while (dc.running) {

          System.out.print("$ ");

          // Saisie utilisateur interruptible
          String cmd = "";
          boolean stopInput = false;
          do {
            try {
              while (!scanner.ready()) {
                if (!dc.running) {
                  stopInput = true;
                  break;
                }
                if (socketStream.ready() && dc.requestContinuously) {
                  try {
                    String updateMessage = dc.recvPacket();
                    dc.majFishesParser(updateMessage);
                  } catch (Exception e) {
                    e.printStackTrace();
                    dc.running = false;
                  }
                }
                Thread.sleep(200);
              }
              if (stopInput) {
                break;
              }
              cmd = scanner.readLine();
            } catch (IOException e) {
              System.err.println("BufferedReader error");
            } catch (InterruptedException e) {
              System.err.println("Error waiting for input");
            }
          } while ("".equals(cmd));

          if (!dc.running) {
            System.err.println("Server disconnected1");
            serverStopped = true;
          } else {
            if (cmd.equals("exit")) {
              dc.running = false;
            } else {
              try {
                dc.sendPacket(cmd);
                String answer = dc.recvPacket();
                // System.out.println(answer);
                dc.handleCommand(cmd, answer);
              } catch (Exception e) {
                System.err.println("Server disconnected");
                dc.running = false;
                serverStopped = true;
              }
            }
          }
        }
      } catch (Exception e) {
        System.err.println("Error setting input stream buffer");
      }
      dc.pingService.shutdown();
      dc.updateService.shutdown();
      if (!serverStopped) {
        dc.endConnection();
      }
      dc.endDisplay();
    }
  }
}
