import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.awt.event.*;
import java.util.*;
import java.util.zip.*;
import java.lang.Math.*;

public class DeltaEncoder 
{
    public static void main(String[] args)
    {
        int     count;
        Display display;

        count = args.length;
        if(count != 1)
            System.out.println("Usage: java DeltaEncoder <filename>");
        else
            display = new Display(args[0]);
    }
}

class Display extends Frame implements ImageObserver
{
    FileOutputStream out;
    Image            image; 
    Dimension        dimension;
    ImageCanvas      image_canvas;
    PixelGrabber     pixel_grabber;
    ColorModel       color_model;
    int              minimum_delta; 
    int              truncated_ratio;
    int              number_of_bytes, size;
    int              number_of_bits;
    int              previous_size, current_size;
    int              maximum_delta, pixel_shift; 
    int              delta[], shrink[]; 
    byte             even_processed_bit_strings[];
    byte             odd_processed_bit_strings[];
    int              x_interval_type[], y_interval_type[];
    int              red[], green[], difference[], blue[], pixel[], original_pixel[];
    int              intermediate_xdim, intermediate_ydim;
    int              xdim, ydim, i, j, k;
    int              even_xdim, even_ydim;
    int              number_of_different_values;
    boolean          errored = false;
    int              cursor_length;

    int              error_amount; 
    Scrollbar        error_scrollbar;
    TextArea         current_error;
    int              error_scrollbar_range, error_position;
    ErrorScrollbarHandler error_handler; 
    Panel            error_panel; 
    int              previous_error = -1;
    int              previous_delta = 10;

    double           zero_one_ratio;
    double           new_zero_one_ratio[];

    Panel            scrollbar_panel; 
    Panel            button_panel; 
    Panel            parameter_panel;

    int              CurrentResolution = 3;
    int              PreviousResolution = -1;
    double           scale_factor;
    int              number_of_iterations;

    DisplayWindowAdapter windowHandler;

    Button           go_button;
    GoButtonHandler  go_handler;
    Button           reload_button;
    ReloadButtonHandler  reload_handler;
    Button           resolution_button;
    ResButtonHandler res_handler;
    TextArea         user_information;
    double           delta_amount_of_compression;
    double           original_amount_of_compression;
    File             image_file;

    public Display(String filename)
    {
        try
        {
            image = Toolkit.getDefaultToolkit().getImage(filename);
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
            xdim = image.getWidth(this);
        }
        ydim = -1;
        while(ydim < 0)
        { 
            ydim = image.getHeight(this); 
        }
        if(xdim % 2 != 0)
            even_xdim = xdim - 1;
        else
            even_xdim = xdim;

        if(ydim % 2 != 0)
            even_ydim = ydim - 1;
        else
            even_ydim = ydim;

        red          = new int[xdim * ydim];
        green        = new int[xdim * ydim];
        difference   = new int[xdim * ydim];
        blue         = new int[xdim * ydim];
        shrink       = new int[xdim * ydim];
        delta        = new int[xdim * ydim];
        original_pixel = new int[xdim * ydim];
        pixel          = new int[xdim * ydim];
        even_processed_bit_strings  = new byte[2 * xdim * ydim];
        odd_processed_bit_strings   = new byte[2 * xdim * ydim];
        x_interval_type = new int[xdim];
        y_interval_type = new int[ydim];
        new_zero_one_ratio = new double[1];

        pixel_grabber = new PixelGrabber(image, 0, 0, xdim, ydim, original_pixel, 0, xdim);
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
        else
        {
            if(even_xdim != xdim || even_ydim != ydim)
                extract(original_pixel, xdim, ydim, 0, 0, original_pixel, even_xdim, even_ydim);
            image = createImage(new MemoryImageSource(even_xdim, even_ydim, color_model, original_pixel, 0, even_xdim));
        }
        image_canvas = new ImageCanvas(image);
        this.add("Center", image_canvas);
        windowHandler = new DisplayWindowAdapter();
        this.addWindowListener(windowHandler);

        user_information = new TextArea("", 1, xdim, TextArea.SCROLLBARS_NONE);
        user_information.setEditable(false);

        go_button  = new Button();
        go_button.setLabel("Delta encode.");
        go_handler = new GoButtonHandler(this);
        go_button.addActionListener(go_handler);

        reload_button  = new Button();
        reload_button.setLabel("Reload.");
        reload_handler = new ReloadButtonHandler(this);
        reload_button.addActionListener(reload_handler);

        resolution_button  = new Button();
        resolution_button.setLabel("    Medium    ");
        res_handler = new ResButtonHandler();
        resolution_button.addActionListener(res_handler);

        error_scrollbar_range = 10;
        error_position        = 0;
        cursor_length = error_scrollbar_range / 5;
        if(cursor_length == 0) 
            cursor_length = 1;
        error_scrollbar = new Scrollbar(Scrollbar.HORIZONTAL, error_position, cursor_length, 0, error_scrollbar_range + cursor_length);
        current_error = new TextArea("", 1, 4, TextArea.SCROLLBARS_NONE);
        current_error.setText(" " + Integer.toString(error_position));
        current_error.setEditable(false);
        error_panel     = new Panel(new BorderLayout());
        error_panel.add("North", new Label("Allowable error"));
        error_panel.add("Center", error_scrollbar);
        error_panel.add("West", current_error);
        error_amount = 0;
        error_handler = new ErrorScrollbarHandler();
        error_scrollbar.addAdjustmentListener(error_handler);

        button_panel = new Panel();
        button_panel.add(go_button);
        button_panel.add(reload_button);
        button_panel.add(resolution_button);

        parameter_panel     = new Panel(new BorderLayout());
        parameter_panel.add("North", button_panel);
        parameter_panel.add("Center", error_panel);
        parameter_panel.add("South", user_information);

        this.add("South", parameter_panel);

        image_file = new File(filename);
        original_amount_of_compression = (double)image_file.length();
        original_amount_of_compression = (1. - (original_amount_of_compression / (xdim * ydim * 3)));
        original_amount_of_compression = (double)((int)(1000. * original_amount_of_compression));
        original_amount_of_compression /= 1000.;
        user_information.setText("Original image...compression is " + original_amount_of_compression);
        this.setSize(even_xdim, even_ydim + 160);
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

    public void GetDeltasFromPixels(int pixel[], int delta[], int xdim, int ydim, int shift, int maximum_delta)
    {
        int i, j, k;
        int current_value;
        int start_value;
        int delta_value;
    
        k = 0;
        start_value = 0;
        for(i = 0; i < ydim; i++)
        {
            delta_value  = (pixel[k] >> shift) - (start_value >> shift);
            start_value += delta_value << shift;
            delta[k]     = delta_value + maximum_delta;
            k++;
            current_value = start_value; 
            for(j = 1; j < xdim; j++)
            {
                delta_value    = (pixel[k] >> shift) - (current_value >> shift);
                current_value += (delta_value << shift);
                delta[k]       = delta_value + maximum_delta;
                k++;
            }
        }
    }
    
    public void mergesort_rank(int value[], int order[], int n)
    {
        int t,u,ut;
        int i,j,k;
        int temp_value, increment;
        int temp_order;
    
        for(i = 0; i < n; i++)
            order[i] = i;
        t = 1;
        while(t < n)
            t <<= 1;
      
        while(t > 1)
        {
            increment = t;
            t >>= 1;
            for(i = 0; i < n; i += increment)
            {
                for(j = 0, k = i; j < t && (k + t < n); j++, k++)
                {
                    if(value[k] < value[k+t])
                    {
                        temp_value = value[k];
                        temp_order = order[k+t];
                        value[k]   = value[k+t];
                        value[k+t] = temp_value;
                        order[k+t] = order[k];
                        order[k]   = temp_order;
                    }
                }
            }
      
            u  = 1;
            ut = t;
            while(ut < n)
            {
                u  <<= 1;
                ut <<= 1;
            }
      
            while(u > 2)
            {
                u  >>= 1;
                ut >>= 1;
                increment = 2 * t;
                for(i = 0; i < n; i += increment)
                {
                    for(j = 0, k = i; j < t && (k + ut < n); j++, k++)
                    {
                        if(value[k+t] < value[k+ut])
                        {
                            temp_value = value[k+t];
                            temp_order = order[k+ut];
    
                            value[k+t]  = value[k+ut];
                            value[k+ut] = temp_value;
                            order[k+ut] = order[k+t];
                            order[k+t]  = temp_order;
                        }
                    }
                }
            }
        }
    }

    public void GetPixelsFromDeltas(int delta[], int pixel[], int xdim, int ydim, int shift, int maximum_delta)
    {
        int current_value;
        int start_value;
        int i, j, k;
    
        k = 0;
        start_value = 0;
        for(i = 0; i < ydim; i++)
        {
            start_value  += (delta[k] - maximum_delta) << shift;
            current_value = start_value;
            pixel[k] = current_value;
            k++;
            for(j = 1; j < xdim; j++)
            {
                current_value += (delta[k] - maximum_delta) << shift;
                pixel[k]       = current_value;
                k++;
            }
        }
    }
    
    public int PackStrings(int src[], int size, int number_of_different_values, byte dst[])
    {
        int i, j, k;
        int current_byte;
        byte current_bit, value; 
        byte next_bit; 
        int  index, second_index;
        int  number_of_bits;
        int inverse_table[], table[], mask[];
    
        inverse_table = new int[number_of_different_values]; 
        inverse_table[0] = number_of_different_values / 2;
        i = j = number_of_different_values / 2;
        k = 1;
        j++;
        i--;
        while(i >= 0)
        {
            inverse_table[k] = j;
            k++;
            j++;
            inverse_table[k] = i;
            k++;
            i--;
        }

        table = new int[number_of_different_values];
        for(i = 0; i < number_of_different_values; i++)
        {
            j        = inverse_table[i];
            table[j] = i;
        }
    
        mask  = new int[8];
        mask[0] = 1;
        for(i = 1; i < 8; i++)
        {
            mask[i] = mask[i - 1];
            mask[i] *= 2;
            mask[i]++;
        }
    
    
        current_bit = 0;
        current_byte = 0;
        dst[current_byte] = 0;
        for(i = 0; i < size; i++)
        {
            k     = src[i];
            index = table[k];
            if(index == 0)
            {
                current_bit++;
                if(current_bit == 8)
                    dst[++current_byte] = current_bit = 0;
            }
            else
            {
                next_bit = (byte)((current_bit + index + 1) % 8);
                if(index <= 7)
                {
                    dst[current_byte] |= (byte) (mask[index - 1] << current_bit);
                    if(next_bit <= current_bit)
                    {
                        dst[++current_byte] = 0;
                        if(next_bit != 0)
                            dst[current_byte] |= (byte)(mask[index - 1] >> (8 - current_bit));
                    }
                }
                else if(index > 7)
                {
                    dst[current_byte] |= (byte)(mask[7] << current_bit);
                    j = (index - 8) / 8;
                    for(k = 0; k < j; k++)
                        dst[++current_byte] = (byte)(mask[7]);
                    dst[++current_byte] = 0;
                    if(current_bit != 0)
                        dst[current_byte] |= (byte)(mask[7] >> (8 - current_bit));
    
                    if(index % 8 != 0)
                    {
                        second_index = index % 8 - 1;
                        dst[current_byte] |= (byte)(mask[second_index] << current_bit);
                        if(next_bit <= current_bit)
                        {
                            dst[++current_byte] = 0;
                            if(next_bit != 0)
                                dst[current_byte] |= (byte)(mask[second_index] >> (8 - current_bit));
                        }
                    }
                    else if(next_bit <= current_bit)
                            dst[++current_byte] = 0;
                }
                current_bit = next_bit;
            }
        }
        if(current_bit != 0)
            current_byte++;
        number_of_bits = current_byte * 8;
        if(current_bit != 0)
            number_of_bits -= 8 - current_bit;
        return(number_of_bits);
    }
    
    public int UnpackStrings(byte src[], int number_of_different_values, int dst[], int size)
    {
        int  number_of_bytes_unpacked, i;
        int  current_src_byte, current_dst_byte;
        byte non_zero, mask, current_bit;
        int  index, current_length;
        int table[];
    
        table = new int[number_of_different_values]; 
        table[0] = number_of_different_values / 2;
        i = j = number_of_different_values / 2;
        k = 1;
        j++;
        i--;
        while(i >= 0)
        {
            table[k] = j;
            k++;
            j++;
            table[k] = i;
            k++;
            i--;
        }
        current_length = 1;
        current_src_byte = 0;
        mask = 0x01;
        current_bit = 0;
        current_dst_byte = 0;
        while(current_dst_byte < size)
        {
            non_zero = (byte)(src[current_src_byte] & (byte)(mask << current_bit));
            if(non_zero != 0)
                current_length++;
            else
            {
                index = current_length - 1;
                dst[current_dst_byte++] = table[index];
                current_length = 1;
            }
            current_bit++;
            if(current_bit == 8)
            {
                current_bit = 0;
                current_src_byte++;
            }
        }
        if(current_bit == 0)
            number_of_bytes_unpacked = current_src_byte;
        else
            number_of_bytes_unpacked = current_src_byte + 1;
        return(number_of_bytes_unpacked);
    }

    public int compressZeroBits(byte src[], int size, byte dst[], double zero_one_ratio[])
    {
        byte mask, current_bit;
        int  current_byte;
        int  number_of_zero_bits;
        int  number_of_bits;
        int  current_length;
        int  i, j, k;
        int  minimum_amount_of_compression, byte_size;

        byte_size    = size / 8;
        minimum_amount_of_compression = 0;
        j = k = number_of_zero_bits = 0;
        current_length = current_byte = 0;
        current_bit = 0;
        mask = 0x01;
        dst[0] = 0;
        for(i = 0; i < size; i++)
        {
        if((current_byte + (byte_size - i / 8) / 2 + minimum_amount_of_compression) > byte_size) 
            return(-1);
            current_length++;
            if((src[k] & (mask << j)) == 0)
            {
                if(current_length == 2)
                {
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
            }
            else
            {
                dst[current_byte] |= (byte)mask << current_bit;
                current_bit++;
                if(current_bit == 8)
                {
                    current_byte++;
                    current_bit = dst[current_byte] = 0;
                }
                if(current_length == 2)
                    dst[current_byte] |= (byte)mask << current_bit;
                else
                    number_of_zero_bits++;
                current_bit++;
                if(current_bit == 8)
                {
                    current_byte++;
                    current_bit = dst[current_byte] = 0;
                }
                current_length = 0;
            }
            j++;
            if(j == 8)
            {
                j = 0;
                k++;
            }
        }
        if(current_length == 1)
        {
            number_of_zero_bits++;
            current_bit++;
            if(current_bit == 8)
            {
                current_byte++;
                current_bit = 0;
            }
        }
        if(current_bit != 0)
            current_byte++;
        number_of_bits = current_byte * 8;
        if(current_bit != 0)
            number_of_bits -= 8 - current_bit;
        zero_one_ratio[0] = (double)number_of_zero_bits;
        zero_one_ratio[0] /= (double)number_of_bits;
        return(number_of_bits);
    }
    
    public int decompressZeroBits(byte src[], int size, byte dst[], double zero_one_ratio[])
    {
        byte mask, current_bit;
        int  current_byte;
        int  current_length;
        int  number_of_zero_bits, number_of_bits;
        int  i, j, k;

        number_of_zero_bits = 0;
        current_length = 0;
        current_byte = 0;
        current_bit  = 0;
        dst[0] = 0;
        mask = 0x01;
        j = k = 0;
        for(i = 0; i < size; i++)
        {
            current_length++;
            if((src[k] & (mask << j)) != 0)
            {
                if(current_length == 2)
                {
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
            }
            else
            {
                if(current_length == 1)
                {
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = 0;
                        dst[current_byte] = 0;
                    }
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = 0;
                        dst[current_byte] = 0;
                    }
                }
                else
                {
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                }
                current_length = 0;
            }
            j++;
            if(j == 8)
            {
                j = 0;
                k++;
            }
        }
        if(current_bit != 0)
            current_byte++;
        number_of_bits = current_byte * 8;
        if(current_bit != 0)
            number_of_bits -= 8 - current_bit;
        zero_one_ratio[0] = (double)number_of_zero_bits;
        zero_one_ratio[0] /= (double)number_of_bits;
        return(number_of_bits);
    }
    
    public int compressOneBits(byte src[], int size, byte dst[], double zero_one_ratio[])
    {
        byte mask, current_bit;
        int  current_byte;
        int  current_length;
        int  i, j, k;
        int  number_of_zero_bits, number_of_bits;
        int  minimum_amount_of_compression, byte_size;

        byte_size    = size / 8;
        minimum_amount_of_compression = 0;
        number_of_zero_bits = 0;
        current_length = current_byte = 0;
        current_bit = 0;
        mask = 0x01;
        j = k = 0;
        dst[0] = 0;
        for(i = 0; i < size; i++)
        {
        if((current_byte + (byte_size - i / 8) / 2 + minimum_amount_of_compression) > byte_size) 
            return(-1);
            current_length++;
            if((src[k] & (mask << j)) != 0)
            {
                if(current_length == 2)
                {
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
            }
            else
            {
                number_of_zero_bits++;
                current_bit++;
                if(current_bit == 8)
                {
                    current_byte++;
                    current_bit = dst[current_byte] = 0;
                }
                if(current_length == 1)
                    dst[current_byte] |= (byte)mask << current_bit;
                else
                    number_of_zero_bits++;
                current_bit++;
                if(current_bit == 8)
                {
                    current_byte++;
                    current_bit = dst[current_byte] = 0;
                }
                current_length = 0;
            }
            j++;
            if(j == 8)
            {
                j = 0;
                k++;
            }
        }
        if(current_bit != 0)
            current_byte++;
        number_of_bits = current_byte * 8;
        if(current_bit != 0)
            number_of_bits -= 8 - current_bit;
        zero_one_ratio[0] = (double)number_of_zero_bits;
        zero_one_ratio[0] /= (double)number_of_bits;
        return(number_of_bits);
    }
    
    public int decompressOneBits(byte src[], int size, byte dst[], double zero_one_ratio[])
    {
        byte mask, current_bit;
        int  current_byte;
        int  current_length;
        int  i, j, k;
        int  number_of_zero_bits, number_of_bits;

        number_of_zero_bits = 0;

        current_length = 0;
        current_byte = 0;
        current_bit  = 0;
        dst[0] = 0;
        mask = 0x01;
        j = k = 0;
        for(i = 0; i < size; i++)
        {
            current_length++;
            if((src[k] & (mask << j)) != 0)
            {
                if(current_length == 2)
                {
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
                else
                {
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
            }
            else
            {
                if(current_length == 2)
                {
                    dst[current_byte] |= (byte)mask << current_bit;
                    current_bit++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_bit++;
                    number_of_zero_bits++;
                    if(current_bit == 8)
                    {
                        current_byte++;
                        current_bit = dst[current_byte] = 0;
                    }
                    current_length = 0;
                }
            }
            j++;
            if(j == 8)
            {
                j = 0;
                k++;
            }
        }
        if(current_bit != 0)
            current_byte++;
        number_of_bits = current_byte * 8;
        if(current_bit != 0)
            number_of_bits -= 8 - current_bit;
        zero_one_ratio[0] = (double)number_of_zero_bits;
        zero_one_ratio[0] /= (double)number_of_bits;
        return(number_of_bits);
    }

    public int compressBits(byte src[], int size, double zero_one_ratio, byte dst[])
    {
        int number_of_iterations, current_size, previous_size;
        int byte_size, type;
        byte mask, temp[];
        double new_zero_one_ratio[];
        
        temp = new byte[size];
        if(zero_one_ratio > .4 && zero_one_ratio < .6)
            return(-1 );
        else
        {
            if(zero_one_ratio <= .4)
                type = 1;
            else
                type = 0;
            byte_size = size / 8;
            if(size % 8 != 0)
                byte_size++;
            new_zero_one_ratio = new double[1];
            new_zero_one_ratio[0] = zero_one_ratio;
            number_of_iterations = 1;
            current_size = previous_size = size;
            if(zero_one_ratio <= .4)
            {
                current_size  = compressOneBits(src, previous_size, dst, new_zero_one_ratio);
                while(current_size < previous_size && current_size > 1)
                {
                    previous_size = current_size;
                    if(new_zero_one_ratio[0] <= .4)
                    {
                        if(number_of_iterations % 2 == 1)
                            current_size = compressOneBits(dst, previous_size, temp, new_zero_one_ratio);
                        else
                            current_size = compressOneBits(temp, previous_size, dst, new_zero_one_ratio);
                        number_of_iterations++;
                    }
                }
            }
            else if(zero_one_ratio >= .6)
            {
                current_size = compressZeroBits(src, previous_size, dst, new_zero_one_ratio);
                while(current_size < previous_size && current_size > 1)
                {
                    previous_size = current_size;
                    if(new_zero_one_ratio[0] >= .6)
                    {
                        if(number_of_iterations % 2 == 1)
                            current_size = compressZeroBits(dst, previous_size, temp, new_zero_one_ratio);
                        else
                            current_size = compressZeroBits(temp, previous_size, dst, new_zero_one_ratio);
                        number_of_iterations++;
                    }
                }
            }
            if(current_size == -1 && number_of_iterations > 1)
            {
                current_size = previous_size;
                number_of_iterations--;
            }
            if(number_of_iterations % 2 == 0)
            {
                byte_size = current_size / 8;
                if(current_size % 8 != 0)
                    byte_size++; 
                for(i = 0; i < byte_size; i++)
                    dst[i] = temp[i];
            }    
        }
        if(current_size > 0)
        {
            System.out.println("The number of iterations was " + number_of_iterations);
            // System.out.println("The type was " + type);
            if(current_size % 8 == 0)
            {
                byte_size = current_size / 8;
                dst[byte_size] = (byte) number_of_iterations;
                if(type == 1)
                    dst[byte_size] |= 128;
                else
                    dst[byte_size] &= 127;
            }
            else
            {
                int  remainder = current_size % 8;
                byte_size = current_size / 8;

                dst[byte_size] |= (byte) (number_of_iterations << remainder);
                byte_size++;
                dst[byte_size] = 0;
                if(remainder > 1)
                    dst[byte_size] = (byte) (number_of_iterations >> 8 - remainder);
                if(type == 1)
                {
                    mask = (byte)1;
                    mask <<= remainder - 1; 
                    dst[byte_size] |= mask;
                }
            }
            return(current_size + 8);
        }
        else
            return(-1);
    }
    
    public int decompressBits(byte src[], int size, byte dst[])
    {
        int  byte_index;
        int  type, number_of_iterations;
        int  size_of_temp, addend;
        int  mask;
        int  remainder, i, even, odd;
        int  previous_size, current_size, byte_size;
        double new_zero_one_ratio[] = new double[1];
        byte temp[];
        
        byte_index = size / 8 - 1;
        remainder  = size % 8;
        if(remainder != 0)
        {
            int value = 254;
            addend = 2;
            for(i = 1; i < remainder; i++)
            {
                value -= addend;
                addend <<= 1;
            } 
            mask = value;
            number_of_iterations = src[byte_index];
            if(number_of_iterations < 0)
                number_of_iterations += 256;
            number_of_iterations &= mask;
            number_of_iterations >>= remainder;
            byte_index++;
            if(remainder > 1)
            {
                mask = 1;
                for(i = 2; i < remainder; i++)
                {
                    mask <<= 1;
                    mask++;
                }
                addend = src[byte_index]; 
                if(addend < 0)
                    addend += 256;
                addend &= mask;
                addend <<= 8 - remainder;
                number_of_iterations += addend;
                mask++;
            }
            else
                mask = 1;
            if((src[byte_index] & mask) == 0)
                type = 0;
            else
                type = 1;
        }
        else
        {
           mask = 127;
           number_of_iterations = src[byte_index];
           if(number_of_iterations < 0)
               number_of_iterations += 256; 
           number_of_iterations &= mask;
           mask++;
           if((src[byte_index] & mask) == 0)
               type = 0;
           else
               type = 1;
        }
        // System.out.println("The number of iterations was " + number_of_iterations);
        // System.out.println("The type was " + type);
        size_of_temp = size;
        for(i = 0; i < number_of_iterations; i++)
            size_of_temp <<= 1;
        temp = new byte[size_of_temp];
        
        current_size = 0;
        if(type == 0)
        {
            if(number_of_iterations == 1)
            {
                current_size = decompressZeroBits(src, size - 8, dst, new_zero_one_ratio);
                number_of_iterations--;
            }
            else if(number_of_iterations % 2 == 0)
            {
                current_size = decompressZeroBits(src, size - 8, temp, new_zero_one_ratio);
                number_of_iterations--;
                while(number_of_iterations > 0)
                {
                    previous_size = current_size;
                    if(number_of_iterations % 2 == 0)
                        current_size = decompressZeroBits(dst, previous_size, temp, new_zero_one_ratio);
                    else
                        current_size = decompressZeroBits(temp, previous_size, dst, new_zero_one_ratio);
                    number_of_iterations--;
                }
            }
            else
            {
                current_size = decompressZeroBits(src, size - 8, dst, new_zero_one_ratio);
                number_of_iterations--;
                while(number_of_iterations > 0)
                {
                    previous_size = current_size;
                    if(number_of_iterations % 2 == 0)
                        current_size = decompressZeroBits(dst, previous_size, temp, new_zero_one_ratio);
            else
                        current_size = decompressZeroBits(temp, previous_size, dst, new_zero_one_ratio);
                    number_of_iterations--;
                }
            }
        }
        else
        {
            if(number_of_iterations == 1)
            {
                current_size = decompressOneBits(src, size - 8, dst, new_zero_one_ratio);
                number_of_iterations--;
            }
            else if(number_of_iterations % 2 == 0)
            {
                current_size = decompressOneBits(src, size - 8, temp, new_zero_one_ratio);
                number_of_iterations--;
                while(number_of_iterations > 0)
                {
                    previous_size = current_size;
                    if(number_of_iterations % 2 == 0)
                        current_size = decompressOneBits(dst, previous_size, temp, new_zero_one_ratio);
                    else
                        current_size = decompressOneBits(temp, previous_size, dst, new_zero_one_ratio);
                    number_of_iterations--;
                }
            }
            else
            {
                current_size = decompressOneBits(src, size - 8, dst, new_zero_one_ratio);
                number_of_iterations--;
                while(number_of_iterations > 0)
                {
                    previous_size = current_size;
                    if(number_of_iterations % 2 == 0)
                        current_size = decompressOneBits(dst, previous_size, temp, new_zero_one_ratio);
                    else
                        current_size = decompressOneBits(temp, previous_size, dst, new_zero_one_ratio);
                    number_of_iterations--;
                }
            }
        }
        return(current_size);
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
        int difference_number_of_bits; 
        int difference_shift;
        int difference_maximum_delta;
        int maximum_remainder, remainder, modulo_value;
        int original_size, byte_size, index; 
        Date start, stop;
        long total;
        double average;
        int string_transform[] = 
        {
            0,
            8, 16, 4, 32, 2, 64, 1, 128,
            10, 20, 40, 80, 36, 18, 72, 34, 68, 66, 65, 130, 129, 24, 12, 48, 6, 96, 9, 17, 33, 132, 136, 144, 5, 160, 3, 192,
            7, 11, 13, 14, 19, 21, 22, 25, 26, 28, 35, 37, 38, 41, 42, 44, 49, 50, 52, 56, 67, 69, 70, 73, 74, 76, 81, 82, 84, 88, 97, 98, 100, 104, 112, 131, 133, 134, 137, 138, 140, 145, 146, 148, 152, 161, 162, 164, 168, 176, 193, 194, 196, 200, 208, 224,
            15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54,  57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240,
            31, 47, 55, 59, 61, 62, 79, 87, 91, 93, 94, 103, 107, 109, 110, 115, 117, 118, 121, 122, 124, 143, 151, 155, 157, 158, 167, 171, 173, 174, 179, 181, 182, 185, 186, 188, 199, 203, 205, 206, 211, 213, 214, 217, 218, 220, 227, 229, 230, 233, 234, 236, 241, 242, 244, 248,
            63, 95, 111, 119, 123, 125, 126, 159, 175, 183, 187, 189, 190, 207, 215, 219, 221, 222, 231, 235, 237, 238, 243, 245, 246, 249, 250, 252,
            127, 191, 223, 239, 247, 251, 253, 254, 
            255
        };
        int reordered_string_transform[] = new int[256];
        int inverse_string_transform[] = new int[256];
        int histogram[]   = new int[256];
        int rank[]        = new int[256]; 
    
        GoButtonHandler(Display display)
        {
            this_display = display;
        }
    
        public void actionPerformed(ActionEvent e)
        {
            if((PreviousResolution != CurrentResolution) ||
                (previous_error != error_amount))
            {
                PreviousResolution = CurrentResolution;
                previous_error     = error_amount;
                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    green[i] = (original_pixel[i] >> 8) & 0xff;        
                }
        
                if(CurrentResolution == 0) 
                {
                    pixel_shift = 6;
                    maximum_delta = 4;
                }
                else if(CurrentResolution == 1)
                {
                    pixel_shift = 5;
                    maximum_delta = 8;
                }
                else if(CurrentResolution == 3)
                {
                    pixel_shift = 4;
                    maximum_delta = 16;
                }
                else if(CurrentResolution == 2)
                {
                    pixel_shift = 3;
                    maximum_delta = 32;
                }
                else if(CurrentResolution == 4)
                {
                    pixel_shift = 2;
                    maximum_delta = 64;
                }
                else if(CurrentResolution == 5)
                {
                    pixel_shift = 1;
                    maximum_delta = 128;
                }

                if(error_amount != 0)
                {
                    scale_factor = 1. - (.033 * (double)error_amount); 
                    intermediate_xdim = (int)((double) even_xdim * scale_factor);
                    intermediate_ydim = (int)((double) even_ydim * scale_factor);
                    initIntervalType(x_interval_type, even_xdim, intermediate_xdim);
                    initIntervalType(y_interval_type, even_ydim, intermediate_ydim);
                    XYtransformDown(green, even_xdim, even_ydim, shrink, intermediate_xdim, intermediate_ydim, x_interval_type, y_interval_type);
                    GetDeltasFromPixels(shrink, delta, intermediate_xdim, intermediate_ydim, pixel_shift, maximum_delta);
                    GetPixelsFromDeltas(delta, shrink, intermediate_xdim, intermediate_ydim, pixel_shift, maximum_delta);

                    initIntervalType(x_interval_type, intermediate_xdim, even_xdim);
                    initIntervalType(y_interval_type, intermediate_ydim, even_ydim);
                    XtransformUp(shrink, intermediate_xdim, intermediate_ydim, delta, even_xdim, x_interval_type);
                    YtransformUp(delta, even_xdim, intermediate_ydim, difference, even_ydim, y_interval_type);
		    for(i = 0; i < even_xdim * even_ydim; i++)
                        difference[i] -= green[i];

                    initIntervalType(x_interval_type, even_xdim, intermediate_xdim);
                    initIntervalType(y_interval_type, even_ydim, intermediate_ydim);
                    XYtransformDown(green, even_xdim, even_ydim, shrink, intermediate_xdim, intermediate_ydim, x_interval_type, y_interval_type);
                    GetDeltasFromPixels(shrink, delta, intermediate_xdim, intermediate_ydim, pixel_shift, maximum_delta);
                    number_of_different_values = 2 * maximum_delta + 1;
                    number_of_bits = PackStrings(delta, intermediate_xdim * intermediate_ydim, number_of_different_values, odd_processed_bit_strings);
                    size = intermediate_xdim * intermediate_ydim;
                    zero_one_ratio = (double) size / (double)number_of_bits;
                    size = (int)(zero_one_ratio * 100.);
                    zero_one_ratio = (double)size / 100.;
                }
                else
                {
                    GetDeltasFromPixels(green, delta, even_xdim, even_ydim, pixel_shift, maximum_delta);
                    GetPixelsFromDeltas(delta, difference, even_xdim, even_ydim, pixel_shift, maximum_delta);
		    for(i = 0; i < even_xdim * even_ydim; i++)
                        difference[i] -= green[i];

                    GetDeltasFromPixels(green, delta, even_xdim, even_ydim, pixel_shift, maximum_delta);
                    number_of_different_values = 2 * maximum_delta + 1;
                    number_of_bits = PackStrings(delta, even_xdim * even_ydim, number_of_different_values, odd_processed_bit_strings);
                    size = even_xdim * even_ydim;
                    zero_one_ratio = (double) size / (double)number_of_bits;
                    size = (int)(zero_one_ratio * 100.);
                    zero_one_ratio = (double)size / 100.;
                }
                System.out.println("The green delta string is " + zero_one_ratio + " zeros.");

                original_size = number_of_bits;
                number_of_bits = compressBits(odd_processed_bit_strings, number_of_bits, zero_one_ratio, even_processed_bit_strings);
                if(number_of_bits != -1)
                {
                    delta_amount_of_compression = number_of_bits / 8;
                    number_of_bits = decompressBits(even_processed_bit_strings, number_of_bits, odd_processed_bit_strings);
                }
                else
                {
                    number_of_bits = original_size;
                    delta_amount_of_compression = number_of_bits / 8;
                }

                if(error_amount != 0)
                {
                    number_of_bytes = UnpackStrings(odd_processed_bit_strings, number_of_different_values, delta, intermediate_xdim * intermediate_ydim);
                    GetPixelsFromDeltas(delta, shrink, intermediate_xdim, intermediate_ydim, pixel_shift, maximum_delta);
                    initIntervalType(x_interval_type, intermediate_xdim, even_xdim);
                    initIntervalType(y_interval_type, intermediate_ydim, even_ydim);
                    XtransformUp(shrink, intermediate_xdim, intermediate_ydim, delta, even_xdim, x_interval_type);
                    YtransformUp(delta, even_xdim, intermediate_ydim, green, even_ydim, y_interval_type);
                }
                else
                {
                    number_of_bytes = UnpackStrings(odd_processed_bit_strings, number_of_different_values, delta, even_xdim * even_ydim);
                    GetPixelsFromDeltas(delta, green, even_xdim, even_ydim, pixel_shift, maximum_delta);
                }

                if(pixel_shift >= 4)
                {
                    shrinkAvg(difference, even_xdim, even_ydim, red);
		    int min = 256;
		    int max = -256;
		    for(i = 0; i < (even_xdim / 2) * (even_ydim / 2); i++)
		    {
		        if(red[i] > max)
		            max = red[i];
		        else
			    if(red[i] < min)
			        min = red[i];
		    }
                    difference_maximum_delta = max - min;
                    difference_shift         = 1;
                    if(difference_maximum_delta > 1)
                    {
                        GetDeltasFromPixels(red, delta, even_xdim / 2, even_ydim / 2, difference_shift, difference_maximum_delta);
                        number_of_different_values = 2 * difference_maximum_delta + 1;
                        difference_number_of_bits = PackStrings(delta, (even_xdim / 2) * (even_ydim / 2), number_of_different_values, odd_processed_bit_strings);
                        size = (even_xdim / 2) * (even_ydim / 2);
                        zero_one_ratio = (double) size / (double)difference_number_of_bits;
                        size = (int)(zero_one_ratio * 100.);
                        zero_one_ratio = (double)size / 100.;
                        System.out.println("The difference delta string is " + zero_one_ratio + " zeros.");
    
                        original_size = difference_number_of_bits;
                        difference_number_of_bits = compressBits(odd_processed_bit_strings, number_of_bits, zero_one_ratio, even_processed_bit_strings);
                        if(difference_number_of_bits != -1)
                        {
                            delta_amount_of_compression += difference_number_of_bits / 8;
                        }
                        else
                        {
                            difference_number_of_bits = original_size;
                            delta_amount_of_compression += difference_number_of_bits / 8;
                        }
                        GetPixelsFromDeltas(delta, red, even_xdim / 2, even_ydim / 2, difference_shift, difference_maximum_delta);
                        expandInterp(red, even_xdim / 2, even_ydim / 2, difference);
                        for(i = 0; i < even_xdim * even_ydim; i++)
                            green[i] -= difference[i];
                    }
                } 

                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    if(green[i] > 255)
                        green[i] = 255;
                    if(green[i] < 0)
                        green[i] = 0;
                }

                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    red[i]   = (original_pixel[i] >> 16) & 0xff;
                    blue[i]  = original_pixel[i] & 0xff;        
                    red[i]  = red[i] - green[i];
                    blue[i] = blue[i] - green[i];
                }
    
                if(CurrentResolution == 0) 
                {
                    pixel_shift = 6;
                    maximum_delta = 8;
                }
                else if(CurrentResolution == 1)
                {
                    pixel_shift = 5;
                    maximum_delta = 16;
                }
                else if(CurrentResolution == 2)
                {
                    pixel_shift = 4;
                    maximum_delta = 32;
                }
                else if(CurrentResolution == 3)
                {
                    pixel_shift = 3;
                    maximum_delta = 64;
                }
                else if(CurrentResolution == 4)
                {
                    pixel_shift = 2;
                    maximum_delta = 128;
                }
                else if(CurrentResolution == 5)
                {
                    pixel_shift = 1;
                    maximum_delta = 256;
                }
                number_of_different_values = 2 * maximum_delta + 1;
                shrinkAvg(red, even_xdim, even_ydim, shrink);

                if(CurrentResolution < 2)
                {
                    GetDeltasFromPixels(shrink, delta, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta); 
                    GetPixelsFromDeltas(delta, difference, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta);
                    expandInterp(difference, even_xdim / 2, even_ydim / 2, delta);
                    for(i = 0; i < even_xdim * even_ydim; i++)
                        delta[i] -= red[i];
                    shrinkAvg(delta, even_xdim, even_ydim, difference);
                    for(i = 0; i < (even_xdim / 2) * (even_ydim / 2); i++)
                        shrink[i] -= difference[i];
                }
    
                GetDeltasFromPixels(shrink, delta, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta);
                number_of_different_values = 2 * maximum_delta + 1;
                number_of_bits = PackStrings(delta, (even_xdim / 2) * (even_ydim / 2), number_of_different_values, odd_processed_bit_strings);
                size = (even_xdim / 2) * (even_ydim / 2);
                zero_one_ratio = (double) size / (double)number_of_bits;
                size = (int)(zero_one_ratio * 100.);
                zero_one_ratio = (double)size / 100.;

                System.out.println("The red-green delta string is " + zero_one_ratio + " zeros.");
                original_size = number_of_bits;
                number_of_bits = compressBits(odd_processed_bit_strings, number_of_bits, zero_one_ratio, even_processed_bit_strings);
                if(number_of_bits != -1)
                {
                    delta_amount_of_compression += number_of_bits / 8;
                    number_of_bits = decompressBits(even_processed_bit_strings, number_of_bits, odd_processed_bit_strings);
                }
                else
                {
                    number_of_bits = original_size;
                    delta_amount_of_compression += number_of_bits / 8;
                }

                number_of_bytes = UnpackStrings(odd_processed_bit_strings, number_of_different_values, delta, (even_xdim / 2) * (even_ydim / 2));
                GetPixelsFromDeltas(delta, shrink, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta);
                expandInterp(shrink, even_xdim / 2, even_ydim / 2, red);
                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    red[i] = red[i] + green[i];
                    if(red[i] < 0)
                        red[i] = 0;
                    else if(red[i] > 255)
                        red[i] = 255;
                }
    
                if(CurrentResolution == 0) 
                {
                    pixel_shift = 6;
                    maximum_delta = 8;
                }
                else if(CurrentResolution == 1)
                {
                    pixel_shift = 5;
                    maximum_delta = 16;
                }
                else if(CurrentResolution == 2)
                {
                    pixel_shift = 4;
                    maximum_delta = 32;
                }
                else if(CurrentResolution == 3)
                {
                    pixel_shift = 3;
                    maximum_delta = 64;
                }
                else if(CurrentResolution == 4)
                {
                    pixel_shift = 2;
                    maximum_delta = 128;
                }
                else if(CurrentResolution == 5)
                {
                    pixel_shift = 1;
                    maximum_delta = 256;
                }
                number_of_different_values = 2 * maximum_delta + 1;
                shrinkAvg(blue, even_xdim, even_ydim, shrink);
                if(CurrentResolution < 2)
                {
                    GetDeltasFromPixels(shrink, delta, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta); 
                    GetPixelsFromDeltas(delta, difference, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta);
                    expandInterp(difference, even_xdim / 2, even_ydim / 2, delta);
                    for(i = 0; i < even_xdim * even_ydim; i++)
                        delta[i] -= blue[i];
                    shrinkAvg(delta, even_xdim, even_ydim, difference);
                    for(i = 0; i < (even_xdim / 2) * (even_ydim / 2); i++)
                        shrink[i] -= difference[i];
                }
             
                GetDeltasFromPixels(shrink, delta, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta);
                number_of_different_values = 2 * maximum_delta + 1;
                number_of_bits = PackStrings(delta, (even_xdim / 2) * (even_ydim / 2), number_of_different_values, odd_processed_bit_strings);
                size = (even_xdim / 2) * (even_ydim / 2);
                zero_one_ratio = (double) size / (double)number_of_bits;
                truncated_ratio = (int)(zero_one_ratio * 100.);
                zero_one_ratio = (double)truncated_ratio / 100.;
                System.out.println("The blue-green delta string is " + zero_one_ratio + " zeros.");

                original_size = number_of_bits;
                number_of_bits = compressBits(odd_processed_bit_strings, number_of_bits, zero_one_ratio, even_processed_bit_strings);
                if(number_of_bits != -1)
                {
                    delta_amount_of_compression += number_of_bits / 8;
                    number_of_bits = decompressBits(even_processed_bit_strings, number_of_bits, odd_processed_bit_strings);
                }
                else
                {
                    number_of_bits = original_size;
                    delta_amount_of_compression += number_of_bits / 8;
                }
                number_of_bytes = UnpackStrings(odd_processed_bit_strings, number_of_different_values, delta, (xdim / 2) * (ydim / 2));
                GetPixelsFromDeltas(delta, shrink, even_xdim / 2, even_ydim / 2, pixel_shift, maximum_delta); 
                expandInterp(shrink, even_xdim / 2, even_ydim / 2, blue);
                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    blue[i] = blue[i] + green[i];
                    if(blue[i] < 0)
                        blue[i] = 0;
                    else if(blue[i] > 255)
                        blue[i] = 255;
                }

                for(i = 0; i < even_xdim * even_ydim; i++)
                {
                    pixel[i] = 0;
                    pixel[i] |= 255 << 24;
                    pixel[i] |= red[i] << 16;
                    pixel[i] |= green[i] << 8;
                    pixel[i] |= blue[i];
                }
                size = even_xdim * even_ydim;

                // delta_amount_of_compression += 1024;
                delta_amount_of_compression = (1. - (delta_amount_of_compression / (3 * size)));
                delta_amount_of_compression = (double)((int)(1000000. * delta_amount_of_compression));
                delta_amount_of_compression /= 1000000.;

                System.out.println("");
                System.out.println("Amount of compression is " + delta_amount_of_compression);
                System.out.println("");

            }
            image = createImage(new MemoryImageSource(even_xdim, even_ydim, color_model, pixel, 0, even_xdim));
            image_canvas.PutImage(image, image_canvas.getGraphics());
            image_canvas.getGraphics().drawImage(image, 0, 0, this_display);
            user_information.setText("Delta encoded image...compression is " + delta_amount_of_compression);
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
            image = createImage(new MemoryImageSource(even_xdim, even_ydim, color_model, original_pixel, 0, even_xdim));
            image_canvas.PutImage(image, image_canvas.getGraphics());
            image_canvas.getGraphics().drawImage(image, 0, 0, this_display);
            user_information.setText("Original image...compression is " + original_amount_of_compression);
        }
    }

    class ResButtonHandler implements ActionListener
    {
        public void actionPerformed(ActionEvent e)
        {
            CurrentResolution++;
            CurrentResolution %= 6;
            if(CurrentResolution == 0)
            {
                resolution_button.setLabel("Very Low ");
            }
            if(CurrentResolution == 1)
            {
                resolution_button.setLabel("Low ");
            }
            else if(CurrentResolution == 2)
            {
                resolution_button.setLabel("Medium-low");
            }
            else if(CurrentResolution == 3)
            {
                resolution_button.setLabel("Medium");
            }
            else if(CurrentResolution == 4)
            {
                resolution_button.setLabel("Medium-high");
            }
            else if(CurrentResolution == 5)
            {
                resolution_button.setLabel("High");
            }
        }
    }

    class ErrorScrollbarHandler implements AdjustmentListener
    {
        int      value;
        
        public void adjustmentValueChanged(AdjustmentEvent event)
        {
            value        = event.getValue();
            error_amount = value;
            current_error.setText(" " + Integer.toString(value));
        }
        
        public void adjustmentValueReset(int new_value)
        {
            current_error.setText(" " + Integer.toString(new_value));
        }
    }
    public void initIntervalType(int interval_type[], int dim, int new_dim)
    {
        int    i, j;
        int    delta; 
        double differential, extra_differential;
        int    length, number, difference;
        int    number_of_intervals;
        int    number_of_long_intervals;
        double real_pixel_index;
        int    start, end;
        int    previous_interval_type;
        int    number_of_runlengths;
        int    current_interval_type;
        int    current_runlength;
    
        delta       = dim - new_dim;
        if(delta < 0)
            delta = -delta;
        number       = delta;    
        length       = dim / number;
        difference   = dim - number * length;
        differential = extra_differential = difference;
        differential /= number;
        extra_differential /= number * number;
    
        i = 0;
        i++;
        real_pixel_index = 0.;
        number_of_intervals = 0;
        number_of_long_intervals = 0;
    
        while(i < dim)
        {
            for(j = 1; j < length -1; j++)
            {
                i++;
            }
            start = (int)real_pixel_index;
            real_pixel_index  += differential;
            end   = (int)real_pixel_index;
            if(end > start)
            {
                i += 2;
                real_pixel_index  += extra_differential;
                interval_type[number_of_intervals + 2] = 1;
                number_of_intervals++;
                number_of_long_intervals++;
            }
            else
            {
                interval_type[number_of_intervals + 2] = 0;
                number_of_intervals++;
                i++;
            }
            if(i < dim)
            {
                i++;
            }
        }
        interval_type[0] = number_of_intervals;
        interval_type[1] = length;
    }

    public void XYtransformDown(int src[], int xdim, int ydim, int dst[], int new_xdim, int new_ydim, int x_interval_type[], int y_interval_type[])
    {
        int i, j, k, m;
        int xend, yend, x, y;
        int xlength, ylength;
        int number_of_xintervals;
        int number_of_yintervals;
    
        k = 0;
        m = 0;
        number_of_xintervals = x_interval_type[0];
        xlength = x_interval_type[1] - 1;
        number_of_yintervals = y_interval_type[0];
        ylength = y_interval_type[1] - 1;
    
        for(i = 0; i < number_of_yintervals; i++)
        {
            yend = ylength + y_interval_type[i + 2];
            for(y = 0; y < yend; y++)
            {
                for(j = 0; j < number_of_xintervals; j++)
                {
                    xend  = xlength + x_interval_type[j + 2];
                    for(x = 0; x < xend; x++)
                        dst[k++] = src[m++];
                    m++;
                }
            }
            m += xdim;
        }
    }

    public void XtransformUp(int src[], int xdim, int ydim, int dst[], int new_xdim, int x_interval_type[])
    {
        int i, j, k, m, n;
        int xend, x, y;
        int xlength;
        int number_of_xintervals;
    
        k = 0;
        m = 0;
        n = 0;
        number_of_xintervals = x_interval_type[0];
        xlength = x_interval_type[1];
    
        for(i = 0; i < ydim; i++)
        {
            for(j = 0; j < number_of_xintervals - 1; j++)
            {
                if(x_interval_type[j + 2] == 1)
                    xend = xlength + 1;
                else
                    xend = xlength;
                for(x = 0; x < xend; x++)
                {
                    dst[k++] = src[m++];
                }
                dst[k++] = (src[m] >> 1) + (src[m + 1] >> 1);
            }
            if(x_interval_type[j + 2] == 1)
                xend = xlength + 1;
            else
                xend = xlength;
            for(x = 0; x < xend; x++)
            {
                dst[k++] = src[m++];
            }
            dst[k++] = src[m];
        }
    }

    public void YtransformUp(int src[], int xdim, int ydim, int dst[], int new_ydim, int y_interval_type[])
    {
        int i, j, k, m, n;
        int yend, x, y;
        int ylength;
        int number_of_yintervals;
    
        k = m = n = 0;
        number_of_yintervals = y_interval_type[0];
        ylength = y_interval_type[1];
    
        for(i = 0; i < number_of_yintervals - 1; i++)
        {
            if(y_interval_type[i + 2] == 1)
                yend = ylength + 1;
            else
                yend = ylength;
            for(y = 0; y < yend; y++)
            {
                for(x = 0; x < xdim; x++)
                {
                    dst[k++] = src[m++];
                }
            }
            for(x = 0; x < xdim; x++)
                dst[k++] = (src[m - xdim + x] >> 1) + (src[m + x] >> 1);
        }
        if(y_interval_type[i + 2] == 1)
            yend = ylength + 1;
        else
            yend = ylength;
        for(y = 0; y < yend; y++)
        {
            for(x = 0; x < xdim; x++)
            {
                dst[k++] = src[m++];
            }
        }
        for(x = 0; x < xdim; x++)
            dst[k++] = src[m - xdim + x];
    }

    public void expandInterp(int src[], int xdim, int ydim, int dst[])
    {
        int       i, j, x, y, new_xdim;
        new_xdim  = 2 * xdim;
        i         = 0; 
        j         = 0;

        // Process the top row.
        dst[j]                = src[i];
        dst[j + 1]            = (13 * src[i] + 7 * src[i + 1]) / 20;
        dst[j + new_xdim]     = (13 * src[i] + 7 * src[i + xdim]) / 20;
        dst[j + new_xdim + 1] = (9  * src[i] + 4 * src[i + 1] + 4 * src[i + xdim] + 3 * src[i + xdim + 1]) / 20;
        i++; 
        j                    += 2;
        for(x = 1; x < xdim - 1; x++)
        {
            dst[j]                = (13 * src[i] + 7 * src[i - 1]) / 20; 
            dst[j + 1]            = (13 * src[i] + 7 * src[i + 1]) / 20; 
            dst[j + new_xdim]     = (9  * src[i] + 4 * src[i - 1] + 4 * src[i + xdim] + 3 * src[i + xdim - 1]) / 20; 
            dst[j + new_xdim + 1] = (9  * src[i] + 4 * src[i + 1] + 4 * src[i + xdim] + 3 * src[i + xdim + 1]) / 20; 
            j += 2;
            i++;
        }
        dst[j]                = (13 * src[i] + 7 * src[i - 1]) / 20;
        dst[j + 1]            = src[i];
        dst[j + new_xdim]     = (9  * src[i] + 4 * src[i - 1] + 4 * src[i + xdim] + 3 * src[i + xdim - 1]) / 20;
        dst[j + new_xdim + 1] = (13 * src[i] + 7 * src[i + xdim]) / 20;
        i++; 
        j                    += new_xdim + 2;

        // Process the middle section.
        for(y = 1; y < ydim - 1; y++)
        {
            dst[j]                = (13 * src[i] + 7 * src[i - xdim]) / 20;
            dst[j + 1]            = (9  * src[i] + 4 * src[i - xdim] + 4 * src[i + 1] + 3 * src[i - xdim + 1]) / 20;
            dst[j + new_xdim]     = (13 * src[i] + 7 * src[i + xdim]) / 20; 
            dst[j + new_xdim + 1] = (9  * src[i] + 4 * src[i + 1] + 4 * src[i + xdim] + 3 * src[i + xdim + 1]) / 20;
            i++;
            j += 2;
            for(x = 1; x < xdim - 1; x++)
            {
                dst[j]                = (9  * src[i] + 4 * src[i - xdim] + 4 * src[i - 1] + 3 * src[i - xdim - 1]) / 20;
                dst[j + 1]            = (9  * src[i] + 4 * src[i - xdim] + 4 * src[i + 1] + 3 * src[i - xdim + 1]) / 20;
                dst[j + new_xdim]     = (9  * src[i] + 4 * src[i + xdim] + 4 * src[i - 1] + 3 * src[i + xdim - 1]) / 20; 
                dst[j + new_xdim + 1] = (9  * src[i] + 4 * src[i + 1] + 4 * src[i + xdim] + 3 * src[i + xdim + 1]) / 20;
                i++;
                j += 2;
            }
            dst[j]                = (9  * src[i] + 7 * src[i - xdim] + 4 * src[i - 1]) / 20;
            dst[j + 1]            = (13 * src[i] + 7 * src[i - xdim]) / 20;
            dst[j + new_xdim]     = (9  * src[i] + 4 * src[i - 1] + 4 * src[i + xdim] + 3 * src[i + xdim - 1]) / 20;
            dst[j + new_xdim + 1] = (13 * src[i] + 7 * src[i + xdim]) / 20; 
            i++;
            j += new_xdim + 2;
        }

        // Process the bottom row.
        dst[j]                = (13 * src[i] + 7 * src[i - xdim]) / 20;
        dst[j + 1]            = (9  * src[i] + 4 * src[i + 1] + 4 * src[i - xdim] + 3 * src[i - xdim + 1]) / 20;
        dst[j + new_xdim]     = src[i];
        dst[j + new_xdim + 1] = (13 * src[i] + 7 * src[i + 1]) / 20;
        i++;
        j                    += 2;
        for(x = 1; x < xdim - 1; x++)
        {
            dst[j]                = (9  * src[i] + 4 * src[i - 1] + 4 * src[i - xdim] + 3 * src[i - xdim - 1]) / 20; 
            dst[j + 1]            = (9  * src[i] + 4 * src[i + 1] + 4 * src[i - xdim] + 3 * src[i - xdim + 1]) / 20; 
            dst[j + new_xdim]     = (13 * src[i] + 7 * src[i - 1]) / 20; 
            dst[j + new_xdim + 1] = (13 * src[i] + 7 * src[i + 1]) / 20; 
            i++;
            j += 2;
        }
        dst[j]                = (9  * src[i] + 4 * src[i - 1] + 4 * src[i - xdim] + 3 * src[i - xdim - 1]) / 20;
        dst[j + 1]            = (13 * src[i] + 7 * src[i - xdim]) / 20;
        dst[j + new_xdim]     = (13 * src[i] + 7 * src[i - 1]) / 20;
        dst[j + new_xdim + 1] = src[i];
    }
    
    public void shrinkAvg(int src[], int xdim, int ydim, int dst[])
    {
        int    i, j, k, r, c;
        int    sum;
    
        r = ydim;
        c = xdim;
        k = 0;
        if(xdim % 2 == 0)
        {
            for (i = 0; i < r - 1; i = i+2)
            {
                for (j = 0; j < c-1; j = j+2)
                {
                    sum= src[i*c+j] + src[i*c+ j+1] + src[(i+1)*c+j]+src[(i+1)*c+j+1];
                    dst[k++] = sum >> 2;
                }
            }
        }
        else
        {
            for (i = 0; i < r - 1; i = i+2)
            {
                for (j = 0; j < c-1; j = j+2)
                {
                    sum= src[i*c+j] + src[i*c+ j+1] + src[(i+1)*c+j]+src[(i+1)*c+j+1];
                    dst[k++] = sum >> 2;
                }
                k++;
            }
        }
    }

    public void extract(int src[], int xdim, int ydim, int xoffset, int yoffset, int dst[], int  new_xdim, int new_ydim)
    {
        int x, y, i, j, xend, yend;
    
        xend = xoffset + new_xdim;
        yend = yoffset + new_ydim;
        j    = 0;
        for(y = yoffset; y < yend; y++)
        {
            i = y * xdim + xoffset;
            for(x = xoffset; x < xend; x++)
            {
                dst[j] = src[i];
                j++;
                i++;
            }
        }
    }
}
