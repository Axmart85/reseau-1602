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
    try {
      Image ocean = ImageIO.read(new File("Display/img/ocean.png"));
      //Display background
      g.drawImage(ocean, 0, 0, this.getWidth(),this.getHeight(), this);

      // Display each fish
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
        } else if (fishes.get(i).getStepY() > 0){
          rotationRequired = - Math.PI/2;
        } else if (fishes.get(i).getStepY() < 0) {
          rotationRequired = Math.PI/2;
        }

        Image img = ImageIO.read(new File("Display/img/poisson.png"));
        Image rezisedimg = img.getScaledInstance((int) dimension.getX(), (int) dimension.getY(), Image.SCALE_DEFAULT);
        BufferedImage img2 = toBufferedImage(rezisedimg);

        //return the fish when goes right
        if (fishes.get(i).getStepX() > 0){
          img2 = calculateVMirror(img2);
        }

        AffineTransform tx = AffineTransform.getRotateInstance(rotationRequired, dimension.getX()/2, dimension.getY()/2);
        AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_BILINEAR);

        // Drawing the rotated image at the required drawing locations
        g.drawImage(op.filter(img2, null), (int) position.getX(), (int) position.getY(), null);
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public BufferedImage calculateVMirror (BufferedImage imgIMG1) {
    BufferedImage result = null ;
    if ( imgIMG1 != null) {
      result = new BufferedImage ( imgIMG1.getWidth() , imgIMG1.getHeight(), BufferedImage.TYPE_4BYTE_ABGR) ;
      for(int col = 0 ; col < imgIMG1.getWidth() ; col++) {
        for( int row = 0 ; row < imgIMG1.getHeight() ; row++) {
          result.setRGB(imgIMG1.getWidth() - col-1, row , imgIMG1.getRGB(col, row ));
        }
      }
    }
    return result;
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
