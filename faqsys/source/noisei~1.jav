import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.awt.event.*;
import java.util.*;

public class NoiseInjecter 
{
    public static void main(String[] args)
    {
        int     count;
        Display display;

        count = args.length;
        if(count != 1)
            System.out.println("Usage: java NoiseInjecter <filename>");
        else
            display = new Display(args[0]);
    }
}

class Display extends Frame implements ImageObserver
{
    Image            original_image, image;
    ImageCanvas      image_canvas;
    PixelGrabber     pixel_grabber;
    ColorModel       color_model;
    int              minimum_delta; 
    int              number_of_bytes, size;
    int              number_of_bits;
    int              maximum_delta, pixel_shift; 
    int              red[], green[], blue[], pixel[], original_pixel[];
    double           mask[];
    int              xdim, ydim, i, j, k;
    int              start_value, number_of_different_values;
    boolean          errored = false;
    int              cursor_length;

    int              random_amount; 
    Scrollbar        random_scrollbar;
    TextArea         current_random;
    int              random_scrollbar_range, random_position;
    RandomScrollbarHandler random_handler; 
    Panel            random_panel; 
    int              previous_random = -1;

    int              weight_amount; 
    Scrollbar        weight_scrollbar;
    TextArea         current_weight;
    int              weight_scrollbar_range, weight_position;
    WeightScrollbarHandler weight_handler; 
    Panel            weight_panel; 
    int              previous_weight = -1;

    int              shift_amount; 
    Scrollbar        shift_scrollbar;
    TextArea         current_shift;
    int              shift_scrollbar_range, shift_position;
    ShiftScrollbarHandler shift_handler; 
    Panel            shift_panel; 
    int              previous_shift = -1;

    Panel            scrollbar_panel; 
    Panel            button_panel; 
    Panel            parameter_panel;

    DisplayWindowAdapter windowHandler;

    Button               go_button;
    GoButtonHandler      go_handler;
    Button               reload_button;
    ReloadButtonHandler  reload_handler;
    TextArea             user_information;
    File                 image_file;

    public Display(String filename)
    {
        try
        {
            original_image = Toolkit.getDefaultToolkit().getImage(filename);
        }
        catch(Exception e)
        {
            System.out.println(e.getMessage());
            System.exit(1);
        }
        color_model = ColorModel.getRGBdefault();
        xdim = -1;
        while(xdim < 0)
        {
            xdim = original_image.getWidth(this);
        }
        ydim = -1;
        while(ydim < 0)
        { 
            ydim = original_image.getHeight(this); 
        }

        red            = new int[xdim * ydim];
        green          = new int[xdim * ydim];
        blue           = new int[xdim * ydim];
        original_pixel = new int[xdim * ydim];
        pixel          = new int[xdim * ydim];
        mask           = new double[xdim * ydim];
        Random random_mask_generator = new Random();
        for(i = 0; i < xdim * ydim; i++)
	    mask[i] = random_mask_generator.nextDouble() + -.5;
       

        pixel_grabber = new PixelGrabber(original_image, 0, 0, xdim, ydim, original_pixel, 0, xdim);
        try
        {
            pixel_grabber.grabPixels();
        }
        catch(InterruptedException e)
        {
            System.err.println(e.toString());
        }
        if((pixel_grabber.getStatus() & ImageObserver.ABORT) != 0)
        {
            System.err.println("Error grabbing pixels.");
            System.exit(1);
        }

        image_canvas = new ImageCanvas(original_image);
        this.add("Center", image_canvas);
        windowHandler = new DisplayWindowAdapter();
        this.addWindowListener(windowHandler);

        user_information = new TextArea("", 1, xdim, TextArea.SCROLLBARS_NONE);
        user_information.setEditable(false);

        go_button  = new Button();
        go_button.setLabel("Quantize.");
        go_handler = new GoButtonHandler(this);
        go_button.addActionListener(go_handler);

        reload_button  = new Button();
        reload_button.setLabel("Reload.");
        reload_handler = new ReloadButtonHandler(this);
        reload_button.addActionListener(reload_handler);

        button_panel = new Panel();
        button_panel.add(go_button);
        button_panel.add(reload_button);

        random_scrollbar_range = 254;
        random_position        = 0;
        cursor_length = random_scrollbar_range / 5;
        if(cursor_length == 0) 
            cursor_length = 1;
        random_scrollbar = new Scrollbar(Scrollbar.HORIZONTAL, random_position, cursor_length, 0, random_scrollbar_range + cursor_length);
        current_random = new TextArea("", 1, 4, TextArea.SCROLLBARS_NONE);
        current_random.setText(" " + Integer.toString(random_position + 1));
        current_random.setEditable(false);
        random_panel     = new Panel(new BorderLayout());
        random_panel.add("North", new Label("Random range"));
        random_panel.add("Center", random_scrollbar);
        random_panel.add("West", current_random);
        random_amount = 1;
        random_handler = new RandomScrollbarHandler(current_random);
        random_scrollbar.addAdjustmentListener(random_handler);

        weight_scrollbar_range = 31;
        weight_position        = 0;
        cursor_length = weight_scrollbar_range / 5;
        if(cursor_length == 0) 
            cursor_length = 1;
        weight_scrollbar = new Scrollbar(Scrollbar.HORIZONTAL, weight_position, cursor_length, 0, weight_scrollbar_range + cursor_length);
        current_weight = new TextArea("", 1, 4, TextArea.SCROLLBARS_NONE);
        current_weight.setText(" " + Integer.toString(weight_position + 3));
        current_weight.setEditable(false);
        weight_panel     = new Panel(new BorderLayout());
        weight_panel.add("North", new Label("Pixel weight"));
        weight_panel.add("Center", weight_scrollbar);
        weight_panel.add("West", current_weight);
        weight_amount = 3;
        weight_handler = new WeightScrollbarHandler(current_weight);
        weight_scrollbar.addAdjustmentListener(weight_handler);

        shift_scrollbar_range = 6;
        shift_position        = 0;
        cursor_length = shift_scrollbar_range / 5;
        if(cursor_length == 0) 
            cursor_length = 1;
        shift_scrollbar = new Scrollbar(Scrollbar.HORIZONTAL, shift_position, cursor_length, 0, shift_scrollbar_range + cursor_length);
        current_shift = new TextArea("", 1, 4, TextArea.SCROLLBARS_NONE);
        current_shift.setText(" " + Integer.toString(shift_position + 1));
        current_shift.setEditable(false);
        shift_panel     = new Panel(new BorderLayout());
        shift_panel.add("North", new Label("Pixel shift"));
        shift_panel.add("Center", shift_scrollbar);
        shift_panel.add("West", current_shift);
        shift_amount = 1;
        shift_handler = new ShiftScrollbarHandler(current_shift);
        shift_scrollbar.addAdjustmentListener(shift_handler);

        scrollbar_panel = new Panel(new BorderLayout());
        Panel top_panel = new Panel(new BorderLayout());
        Panel bottom_panel = new Panel(new BorderLayout());
        top_panel.add("North", button_panel);
        top_panel.add("South", random_panel);
        bottom_panel.add("North", weight_panel);
        bottom_panel.add("South", shift_panel);
        scrollbar_panel.add("North", top_panel);
        scrollbar_panel.add("South", bottom_panel);

        parameter_panel     = new Panel(new BorderLayout());
        parameter_panel.add("Center", scrollbar_panel);
        parameter_panel.add("South", user_information);

        this.add("South", parameter_panel);

        image_file = new File(filename);
        user_information.setText("Original image with no quantizing or noise injection.");
        this.setSize(xdim, ydim + 280);
        this.setVisible(true);
    }

    public boolean imageUpdate(Image image, int infoflags, int x, int y, int w, int h)
    {
        if((infoflags & (ERROR)) != 0)
            errored = true;
        boolean done = ((infoflags & (ERROR | FRAMEBITS | ALLBITS)) != 0);
        repaint(done ? 0 : 100);
        return !done;
    }

    class ImageCanvas extends Canvas
    {
        Image this_image;

        public ImageCanvas(Image image)
        {
            this_image = image;
        }
    
        public synchronized void PutImage(Image image, Graphics g)
        {
            this_image = image;
        }
    
        public synchronized void paint(Graphics g)
        {
            g.drawImage(this_image, 0, 0, this);
        }
    }

    //The following listener is used to terminate the 
    // program when the user closes the Frame object.
    class DisplayWindowAdapter extends WindowAdapter
    {
        public void windowClosing(WindowEvent e)
        {
            System.exit(0);
        }
    }   
    
    class GoButtonHandler implements ActionListener
    {
        Display this_display;
    
        GoButtonHandler(Display display)
        {
            this_display = display;
        }

    
        public void actionPerformed(ActionEvent e)
        {
            if((previous_random != random_amount) || (previous_weight != weight_amount) || (previous_shift != shift_amount))
            {
                previous_random = random_amount;
                previous_weight = weight_amount;
                previous_shift  = shift_amount;
                System.out.println("The random amount is " + random_amount);
                System.out.println("The weight is " + weight_amount);
                System.out.println("The shift is " + shift_amount);
                for(i = 0; i < xdim * ydim; i++)
                {
                    red[i]   = (original_pixel[i] >> 16) & 0xff;
                    red[i] >>= shift_amount;
                    red[i] <<= shift_amount;
                    green[i] = (original_pixel[i] >> 8) & 0xff;        
                    green[i] >>= shift_amount;
                    green[i] <<= shift_amount;
                    blue[i]  = original_pixel[i] & 0xff;        
                    blue[i] >>= shift_amount;
                    blue[i] <<= shift_amount;
                }

                i = xdim + 1;
                for(k = 0; k < ydim - 1; k++)
                { 
		    for(j = 1; j < xdim; j++)
		    {
                        int offset = 0;
                        int random_addend = (int)(mask[i] * (double)(random_amount));
		        red[i] += random_addend;
                        if(red[i] < 0)
                            red[i] = 0;
                        else if(red[i] > 255)
                            red[i] = 255;
			red[i] *= weight_amount;
			red[i] += 2 * red[i - (xdim - 1)]; 
			red[i] += 2 * red[i - xdim]; 
			red[i] += 3 * red[i - 1]; 
			red[i] /= 8 + weight_amount; 
			offset += (xdim * ydim) / 3;
                        random_addend = (int)(mask[(i + offset) % (xdim * ydim)] * (double)random_amount);  
		        green[i] += random_addend;
                        if(green[i] < 0)
                            green[i] = 0;
                        else if(green[i] > 255)
                            green[i] = 255;
			green[i] *= weight_amount;
			green[i] += 2 * green[i - (xdim - 1)]; 
			green[i] += 2 * green[i - xdim]; 
			green[i] += 3 * green[i - 1]; 
			green[i] /= 8 + weight_amount; 
			offset += (xdim * ydim) / 3;
		        random_addend = (int)(mask[(i + offset) % (xdim * ydim)] * (double)random_amount); 
                        blue[i] += random_addend;
                        if(blue[i] < 0)
                            blue[i] = 0;
                        else if(blue[i] > 255)
                            blue[i] = 255;
			blue[i] *= weight_amount;
			blue[i] += 2 * blue[i - (xdim - 1)]; 
			blue[i] += 2 * blue[i - xdim]; 
			blue[i] += 3 * blue[i - 1]; 
			blue[i] /= 8 + weight_amount; 
			i++;
		    }
		    i++;
		}

                for(i = 0; i < xdim * ydim; i++)
                {
                    pixel[i] = 0;
                    pixel[i] |= 255 << 24;
                    pixel[i] |= red[i] << 16;
                    pixel[i] |= green[i] << 8;
                    pixel[i] |= blue[i];
                }
                image = createImage(new MemoryImageSource(xdim, ydim, color_model, pixel, 0, xdim));
            }

            image_canvas.PutImage(image, image_canvas.getGraphics());
            image_canvas.getGraphics().drawImage(image, 0, 0, image_canvas);
            user_information.setText("Quantized image with noise injection.");
        }
    }

    class ReloadButtonHandler implements ActionListener
    {
        Display this_display;
    
        ReloadButtonHandler(Display display)
        {
            this_display = display;
        }
    
        public void actionPerformed(ActionEvent e)
        {
           image_canvas.PutImage(original_image, image_canvas.getGraphics());
           image_canvas.getGraphics().drawImage(original_image, 0, 0, this_display);
           user_information.setText("Original unprocessed image.");
        }
    }

    class RandomScrollbarHandler implements AdjustmentListener
    {
        TextArea value_box;
        int      value;
        
        RandomScrollbarHandler(TextArea text_area)
        {
            value_box = text_area;
        }
        
        public void adjustmentValueChanged(AdjustmentEvent event)
        {
            value = event.getValue();
            random_amount = value + 1;
            value_box.setText(" " + Integer.toString(value + 1));
        }
        
        public void adjustmentValueReset(int new_value)
        {
            value_box.setText(" " + Integer.toString(new_value + 1));
        }
    }

    class WeightScrollbarHandler implements AdjustmentListener
    {
        TextArea value_box;
        int      value;
        
        WeightScrollbarHandler(TextArea text_area)
        {
            value_box = text_area;
        }
        
        public void adjustmentValueChanged(AdjustmentEvent event)
        {
            value = event.getValue();
            weight_amount = value + 3;
            value_box.setText(" " + Integer.toString(value + 3));
        }
        
        public void adjustmentValueReset(int new_value)
        {
            value_box.setText(" " + Integer.toString(new_value + 3));
        }
    }

    class ShiftScrollbarHandler implements AdjustmentListener
    {
        TextArea value_box;
        int      value;
        
        ShiftScrollbarHandler(TextArea text_area)
        {
            value_box = text_area;
        }
        
        public void adjustmentValueChanged(AdjustmentEvent event)
        {
            value = event.getValue();
            shift_amount = value + 1;
            value_box.setText(" " + Integer.toString(value + 1));
        }
        
        public void adjustmentValueReset(int new_value)
        {
            value_box.setText(" " + Integer.toString(new_value + 1));
        }
    }
}
