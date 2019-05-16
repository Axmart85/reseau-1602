import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.Point;
import java.util.ArrayList;
import java.util.List;

public class Client{
  public List<Fish> fishes = new ArrayList<>();

  public void parser(String buffer){
    String[] strings = buffer.split(";");
    int n = strings.length;
    System.out.println(n);
    for (int i = 0; i < n; i++) {
      String[] fishes_value = strings[i].split(",");
      String name = fishes_value[0];
      String mobility = "RandomWayPoint";
      int xpos = Integer.parseInt(fishes_value[1]);
      int ypos = Integer.parseInt(fishes_value[2]);
      int xdim = Integer.parseInt(fishes_value[3]);
      int ydim = Integer.parseInt(fishes_value[4]);
      fishes.add(new Fish(name, mobility, xpos, ypos, xdim, ydim));
      System.out.println("Fish numero " + i);
      System.out.println(name);
      System.out.println(mobility);
      System.out.println(xpos);
      System.out.println(ypos);
      System.out.println(xdim);
      System.out.println(ydim);
    }
  }

  private void fishesParser(String buffer){
    String rmlist = buffer.substring(4,buffer.length());
    System.out.println(rmlist);

    String[] strings = rmlist.split("]");
    int nbFishes = strings.length;

    for (int i = 0; i < nbFishes; i++) {
      System.out.println(strings[i]);

      String s = strings[i].substring(2,strings[i].length());
      System.out.println(s);

      String[] fishes_value = s.split(",");
      String[] nameAndPos = fishes_value[0].split(" at ");
      String name = nameAndPos[0];
      System.out.println(name);
      String mobility = "RandomWayPoint";
      String[] pos = nameAndPos[1].split("x");
      String[] dim = fishes_value[1].split("x");
      int xpos = Integer.parseInt(pos[0]);
      int ypos = Integer.parseInt(pos[1]);
      int xdim = Integer.parseInt(dim[0]);
      int ydim = Integer.parseInt(dim[1]);
      int timetoarrived = Integer.parseInt(fishes_value[2]);
      fishes.add(new Fish(name, mobility, xpos, ypos, xdim, ydim));
      System.out.println("Fish numero " + i);
      System.out.println(name);
      System.out.println(mobility);
      System.out.println(xpos);
      System.out.println(ypos);
      System.out.println(xdim);
      System.out.println(ydim);
    }
  }

  private void addFishesParser(String buffer){
    String rmaddFish = buffer.substring(8,buffer.length());
    String[] fishes_value = rmaddFish.split(", ");
    String[] nameAndPos = fishes_value[0].split(" at ");
    String name = nameAndPos[0];
    String[] pos = nameAndPos[1].split("x");
    String[] dim = fishes_value[1].split("x");
    int xpos = Integer.parseInt(pos[0]);
    int ypos = Integer.parseInt(pos[1]);
    int xdim = Integer.parseInt(dim[0]);
    int ydim = Integer.parseInt(dim[1]);
    String mobility = fishes_value[2].substring(0, fishes_value[2].length());
    fishes.add(new Fish(name, mobility, xpos, ypos, xdim, ydim));
  }

  public static void main(String[] args){

    Client c = new Client();
    String cmd = new String("addFish PoissonNain at 10x10, 4x4, RandomWayPoint");

    c.addFishesParser(cmd);

  }
}
