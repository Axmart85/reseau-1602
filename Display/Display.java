import java.io.*;
import java.net.*;
import javax.swing.*;
import java.util.*;
import java.awt.Point;
import java.awt.*;
import java.awt.event.*;


public class Display extends JFrame implements Runnable{
  // static final int port = 8888;
  public Vue vue;
  private volatile boolean running = true;
  public int refreshTime = 5; //en milisecondes
  private int vueXSize;
  private int vueYSize;

  public Display(int dimX, int dimY) {
    vueXSize = dimX;
    vueYSize = dimY;
    this.vue = new Vue(vueXSize, vueYSize);
    initDisplay();
    // System.out.println(this.vue.getWidth());
    // System.out.println(this.vue.getHeight());
  }

  public void run(){
    move();
  }

  public void terminate(){
    running = false;
  }

  public void addFish(Fish f){
    vue.addFish(f);
  }

  public void removeFish(Fish f){
    vue.removeFish(f);
  }

  public void moveFishes(){
    vue.moveFishes();
  }

  public void initDisplay(){
    this.setTitle("Aquarium");
    this.setSize(vueXSize, vueYSize);
    this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
    this.setLocationRelativeTo(null);
    this.setContentPane(vue);
    this.setVisible(true);
    this.setResizable(false);
  }

  public void majVue(Vue v){
    this.vue = v;
    this.vue.repaint();
  }

  private void move() {
    while(running){

      vue.moveFishes();

      //On redessine notre Display
      vue.repaint();

      try {
        Thread.sleep(refreshTime);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    this.dispatchEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
  }
}
