import java.net.Socket;

public class Updater implements Runnable{
    private Socket sock;
    private Display display;
    private volatile boolean running = true;

    public Updater(Socket sock, Display display){
        this.sock = sock;
        this.display = display;
    }

    private void majFishesParser(String buffer) { // suppression et ajout poisson a discuter
        boolean isStatus = buffer.startsWith("stat");
        String rmlist = buffer.substring(4, buffer.length());
        String[] strings = rmlist.split("]");
        int nbFishes = strings.length;
    
        if (isStatus) {
          System.out.println(
              "\t->OK : Connecté au contrôleur, " + nbFishes + (nbFishes > 1 ? " poissons trouvés" : " poisson trouvé"));
        }
    
        for (int i = 0; i < nbFishes; i++) {
          String s = strings[i].substring(2, strings[i].length());
    
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
                if ((int) prevDest.getX() != destx && (int) prevDest.getY() != desty) {
                  display.vue.fishes.get(j).setPosition(xpos, ypos);
                  Point desti = new Point(destx, desty); // normalement destination donner par controleur
                  display.vue.fishes.get(j).set_move(desti, timetoarrived, display.refreshTime);
                  break;
                }
              }
            }
            if (!found) {
              display.vue.addFish(new Fish(name, "RandomWayPoint", xpos, ypos, 60, 60));
            }
          }
        }
      }

    public void terminate(){
        running = false;
    }

    public void run(){
        while (running){
            
        }
    }
}