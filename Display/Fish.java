import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.Point;

public class Fish{
  private String name;
  private String mobilite;
  public Point position;
  private Point dimension;
  private Point destination;
  private float stepx;
  private float stepy;
  private Point initposition;
  private int numbermoves;
  private boolean started;

  public Fish(String name, String mobilite){
    this.name = name;
    this.mobilite = mobilite;
    this.position = new Point(0,0);
    this.destination = this.position;
    this.dimension = new Point(0,0);
    this.started = false;
  }

  public Fish(String name, String mobilite, int xpos, int ypos, int xdim, int ydim){
    this.name = name;
    this.mobilite = mobilite;
    this.position = new Point(xpos,ypos);
    this.destination = this.position;
    this.dimension = new Point(xdim,ydim);
    this.destination = this.position;
    this.initposition = this.position;
    this.numbermoves = 0;
    this.stepx = 0;
    this.stepy = 0;
    this.started = false;
  }

  public String getName(){
    return this.name;
  }

  public void setName(String name){
    this.name = name;
  }

  public String getMobilite(){
    return this.mobilite;
  }

  public void setMobilite(String mobilite){
    this.mobilite = mobilite;
  }

  public Point getPosition(){
    return this.position;
  }

  public void setPosition(int x, int y){
    this.position = new Point(x, y);
  }

  public Point getDimension(){
    return this.dimension;
  }

  public void setDimension(int x, int y){
    this.dimension = new Point(x, y);
  }

  public Point getDestination(){
    return this.destination;
  }

  public void setDestination(int x, int y){
    this.destination = new Point(x, y);
  }

  public void setDestination(Point dest){
    this.destination = dest;
  }

  public void startFish(){
    this.started = true;
  }

  public boolean isStarted(){
    return this.started;
  }

  public boolean arrived(){
    return (this.position.equals(this.destination));
  }

  public float getStepX(){
    return this.stepx;
  }

  public float getStepY(){
    return this.stepy;
  }

//time en sec et refresh_time en milisec
  public void set_move(Point dest, int time, int refresh_time){
    this.setDestination(dest);
    float refreshInSec = (float) refresh_time * (float) 0.001;
    this.initposition = this.position;
    this.numbermoves = 0;
    int nbsteps = (int) ((float) time/refreshInSec);
    this.stepx = (float)(dest.getX() - this.position.getX()) / nbsteps;
    this.stepy = (float)(dest.getY() - this.position.getY()) / nbsteps;
  }

  public void move(){
    if (this.started && ! this.arrived()){
      this.numbermoves++;
      int new_x = (int)(this.initposition.getX() + this.numbermoves * this.stepx);
      int new_y = (int)(this.initposition.getY() + this.numbermoves * this.stepy);
      setPosition(new_x, new_y);
    }
  }


}
