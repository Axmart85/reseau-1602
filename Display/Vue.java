import java.awt.*;
import javax.swing.*;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import java.util.ArrayList;
import java.util.List;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;

public class Vue extends JPanel {

  private int dimX;
  private int dimY;
  public List<Fish> fishes = new ArrayList<>();

  public Vue(int x, int y){
    dimX = x;
    dimY = y;
  }

  public void paintComponent(Graphics g){
    // On choisit une couleur de fond pour le rectangle //
    //g.setColor(Color.white);
    // On le dessine de sorte qu'il occupe toute la surface //
    //g.fillRect(0, 0, this.getWidth(), this.getHeight());

    try {
      Image ocean = ImageIO.read(new File("Display/img/ocean.png"));
      //Pour une image de fond
      g.drawImage(ocean, 0, 0, this.getWidth(),this.getHeight(), this);
      // g.drawImage(img, posX, posY, dimX, dimY, this);
      for (int i=0; i<fishes.size() ;i++) {
        double factorX = (double) this.getWidth()/dimX;
        double factorY = (double) this.getHeight()/dimY;
        Point relativePosition = fishes.get(i).getPosition();
        Point relativeDimension = fishes.get(i).getDimension();
        Point dimension = new Point((int) (relativeDimension.getX()* factorX), (int) (relativeDimension.getY()* factorY));
        Point position = new Point((int)(relativePosition.getX()* factorX), (int)(relativePosition.getY()* factorY));

        // Rotation information

        double rotationRequired = 0;
        if(fishes.get(i).getStepX() != 0){
          rotationRequired = Math.atan(fishes.get(i).getStepY() / fishes.get(i).getStepX());
        }

        Image img;

        if (fishes.get(i).getStepX() > 0){
          img = ImageIO.read(new File("Display/img/poisson_reverse.png"));
        } else {
          img = ImageIO.read(new File("Display/img/poisson.png"));
        }

        Image rezisedimg = img.getScaledInstance((int) dimension.getX(), (int) dimension.getY(), Image.SCALE_DEFAULT);
        AffineTransform tx = AffineTransform.getRotateInstance(rotationRequired, dimension.getX()/2, dimension.getY()/2);
        AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_BILINEAR);
        BufferedImage img2 = toBufferedImage(rezisedimg);

        // Drawing the rotated image at the required drawing locations
        g.drawImage(op.filter(img2, null), (int) position.getX(), (int) position.getY(), null);
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public static BufferedImage toBufferedImage(Image img){
    if (img instanceof BufferedImage)
    {
        return (BufferedImage) img;
    }

    // Create a buffered image with transparency
    BufferedImage bimage = new BufferedImage(img.getWidth(null), img.getHeight(null), BufferedImage.TYPE_INT_ARGB);

    // Draw the image on to the buffered image
    Graphics2D bGr = bimage.createGraphics();
    bGr.drawImage(img, 0, 0, null);
    bGr.dispose();

    // Return the buffered image
    return bimage;
  }

  public int getDimX() {
    return dimX;
  }

  public void setDimX(int dimX) {
    this.dimX = dimX;
  }

  public int getDimY() {
    return dimY;
  }

  public void setDimY(int dimY) {
    this.dimY = dimY;
  }

  public void addFish(Fish f){
    this.fishes.add(f);
  }

  public void removeFish(Fish f){
    this.fishes.remove(f);
  }

  public void moveFishes(){
    for (int i = 0; i<fishes.size() ; i++ ) {
      fishes.get(i).move();
    }
  }

}
