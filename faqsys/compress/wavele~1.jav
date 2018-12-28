import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.awt.event.*;

public class WaveletCompressor 
{
    void daub4(int a[], int offset, int n, int min_n)
    {
        int  temp[] = new int[640];
        int  nh, nh1;
        int  i, j;
        double c0 = 0.4829629; 
        double c1 = 0.8365163;
        double c2 = 0.2241438;
        double c3 = -0.12940;
    
        if(n < min_n) 
            return;

    	nh1 = (nh = n >> 1) + 1; 
    	nh  = nh - 1;
    	nh1 = nh1 - 1;
    
    	for(i = 0, j = 0; j <= n - 4; j += 2, i++)
    	{
    	    temp[i] = (int)(c0 * a[j + offset] + c1 * a[j + 1 + offset] + c2 * a[j + 2 + offset] + c3 * a[j + 3 + offset]);
    	    temp[i + nh1] = (int)(c3 * a[j + offset] - c2 * a[j + 1 + offset] + c1 * a[j + 2 + offset] - c0 * a[j + 3 + offset]);
    	}
    	temp[i] = (int)(c0 * a[n - 2 + offset] + c1*a[n - 1 + offset] + c2 * a[offset] + c3 * a[1 + offset]);
    	temp[i + nh1] = (int)(c3 * a[n - 2 + offset] - c2 * a[n - 1 + offset] + c1 * a[offset] - c0 * a[1 + offset]);
        for(i = 0; i < n; i++)
            a[i + offset] = temp[i];
     }

     void transpose(int array[], int xdim, int ydim)
     {
         int temp[];
         int i,j;
         int k = 0;
         int rows, columns;
         
	     temp = new int[xdim * ydim];
	     for(i = 0; i < xdim * ydim; i++)
		     temp[i] = 0;
         rows = ydim;
         columns = xdim;
         
         for(i = 0; i < (columns / 2); i++)
         {
             for (j = 0; j < rows; j++)
             {
                temp[k++] = array[j * columns + i];
             }
         } 
         for(i = 0; i < xdim * ydim; i++)
             array[i] = temp[i];
     }
        
     void forward_xform(int src[], int xdim, int ydim)
     {
         int  i, stop, offset;
         int nn;
     
         stop = xdim;
         while((stop % 2) == 0)
             stop = stop / 2;
         stop = stop * 2;
     
         for(i = 0; i < ydim; i++)  
         {
             offset = i * xdim;
             for(nn = xdim; nn >= stop; nn >>= 1)
                 daub4(src, offset, nn, stop);
         }
     
         transpose(src, xdim, ydim);
     
         stop = ydim;
         while((stop % 2) == 0)
             stop = stop / 2;
         stop = stop * 2;
     
         for(i = 0; i < xdim / 2; i++)  
         {
             offset = i * ydim;
             for(nn = ydim; nn >= stop; nn >>= 1)
                 daub4(src, offset, nn, stop);
         }
    }

    int sg_pack(int src, byte dst[], int start_byte[], int start_bit[], int num_of_pack_bits)
    {
        int bits_mask[] = new int[9];
        int sum, addend, index;
        int i;

        /* This should be passed to function, so it only */
        /* has to be done once.                          */ 
        sum    = 0;
        addend = 1;
        for(i = 0; i < 9; i++)
        {
            bits_mask[i] = sum;
    	    sum         += addend;
    	    addend      *= 2;
        }

        int num_of_bytes = 0;		 

        byte byte_value = 0;	                           

        int byte_length = 8;
        int num_of_bits_left_in_byte = 0; 
        int num_of_remaining_bits = num_of_pack_bits;    

        num_of_bytes = (num_of_pack_bits + start_bit[0]) / byte_length;

        for (i = 0; i < num_of_bytes; i++) 
        {
            num_of_bits_left_in_byte = byte_length - start_bit[0];

            byte_value = (byte)(src & bits_mask[num_of_bits_left_in_byte]);

            src >>= num_of_bits_left_in_byte;

            byte_value <<= start_bit[0];

            index = start_byte[0];
            dst[index] = (byte)((dst[index] << num_of_bits_left_in_byte) >> num_of_bits_left_in_byte); 

            dst[index] |= byte_value;

            start_byte[0] += 1;
            start_bit[0]   = 0;
            num_of_remaining_bits -= num_of_bits_left_in_byte;
        }
  
        if(num_of_remaining_bits > 0) 
        {
            num_of_bits_left_in_byte = byte_length - start_bit[0];

            byte_value = (byte)(src & bits_mask[num_of_remaining_bits]);

            src >>= num_of_remaining_bits;

            byte_value <<= start_bit[0];

            index = start_byte[0];
            dst[index] = (byte)((dst[index] << num_of_bits_left_in_byte) >> num_of_bits_left_in_byte);

            dst[index] |= byte_value;

            start_bit[0] += num_of_remaining_bits;
            num_of_remaining_bits = 0;
        }
        return(start_byte[0]);
    }
        
    public void threshold(int src[], int size, double amount_of_compression)
    {
        int    i,j,k;
        int    tmp;
        int    maximum, count;
        double sum;
        int    hist[];
    
        maximum = 0;
        for (i = 0; i < size / 2; i++)       
        {
            tmp = src[i];
            if(tmp < 0)
                tmp = -tmp;
            if(tmp > maximum)
                maximum = tmp;
        }
        
        hist = new int[maximum +1];
        for(i = 0; i < maximum + 1; i++)
           hist[i] = 0; 
    
        count = 0;
        for(i = 0; i < size / 2; i++)
        {
			k = Math.abs(src[i]);
    	    hist[k]++;
    	    count++;
        }
	k = 0;
        sum = 0;
        while(sum < amount_of_compression)
        {
    	    sum += (double) hist[k] / (double) count;
	    k++;
        }
    
        for (i = 0; i < size / 2; i++)       
        {
	    if(Math.abs(src[i]) < k)
		src[i] = 0;
        }
    }
        

    public int pack(int src[], int xdim, int ydim, byte dst[])
    {
        int min, max, number_of_nonzero_values;
        int product, power_of_two;
        int range_of_values, number_of_different_values, addend;
        int is_a_value[], histogram[], sortarray[];
        int maxsize;  
        int occupybits;
        int data_size;
        int value_size;
        int map[];
    	int NumberOfBytes;

        byte data[];  
        byte value[]; 
    	byte byte_value;
      
        int i, j, k;
        int current_iblock, current_jblock, cnt, numholder; 
        int current_data_position, current_value_position; 
        int current_i, current_j;
        int upper_i, upper_j;
        int totalpt;
        int curblock;
        int iblock, jblock;
        int totaldata, totalvalue;
        int start_byte[] = new int[1]; 
		start_byte[0]    = 0;
        int start_bit[]  = new int[1];
		start_bit[0]     = 0;
      
        current_i = current_j = 0;
        iblock = (int)Math.ceil((double) ydim / 16);
        jblock = (int)Math.ceil((double) xdim / 16);
      
        max = min = src[0];
        number_of_nonzero_values = 0;
        for(i = 0; i < xdim * ydim; i++)
        {
            j = src[i];
            if(j != 0)
                number_of_nonzero_values++;
            if(j < min)
                min = j;
            else
                if(j > max)
                    max = j;
        }
        
        range_of_values = max + Math.abs(min) + 1;    
      
        histogram = new int[range_of_values];
        is_a_value = new int[range_of_values];
      
        addend = Math.abs(min);
        number_of_different_values = 0;
        for(i = 0; i < xdim * ydim; i++)
        {
            j = src[i] + addend;
            if(is_a_value[j] == 0) 
            {
                is_a_value[j] = 1;
                number_of_different_values++;
            }
        }
        sortarray = new int[number_of_different_values];
        for (i = 0, j = 0; i < range_of_values; i++) 
        {
            if(is_a_value[i] != 0)
                sortarray[j++] = i;
        }
        product = 1;
        power_of_two = 0;
        while(product < number_of_different_values)
        {
            product *= 2;
            power_of_two++;
        }
        occupybits = power_of_two;
        map = new int[number_of_different_values + 1]; 
        maxsize = number_of_different_values;
      
        for (i = 0; i < number_of_different_values; i++) 
        {
            histogram[sortarray[i]] = i;
            map[i] = sortarray[i];
        }

        /* Tells how much to bias. */
        map[number_of_different_values] = addend;        
      
        totaldata = number_of_nonzero_values + (iblock * jblock);
        totalvalue = (int)(Math.ceil(((double) number_of_nonzero_values * occupybits) / 8));
      
        data_size  = totaldata;
        value_size = totalvalue;
        data       = new byte[totaldata];
        value      = new byte[totalvalue]; 
      
        current_data_position = 1;
        
        numholder = 0;
        for (current_iblock = 0; current_iblock < iblock; current_iblock++) 
        {
            for(current_jblock = 0; current_jblock < jblock; current_jblock++) 
            {
                cnt = 0;
                for(i = 0; i < 16; i++)
                {
                    for(j = 0; j < 16; j++) 
                    {
                        current_i = current_iblock * 16 + i;
                        current_j = current_jblock * 16 + j;
                        if(!((current_i < ydim) && (current_j < xdim)))
                            continue;
                        k = src[current_i *  xdim + current_j];
                        if(k != 0) 
                        {
                            k += addend;    
                            data[current_data_position++] = (byte)(((i & 0xF) << 4) | (j & 0xF)); 
                            sg_pack(histogram[k], value, start_byte, start_bit, occupybits);
                            cnt++;
                        }
                    }
                }
                if(cnt == 256) 
                {
                    current_data_position -= 1;
                    cnt = 255;
                }
                data[numholder] = (byte)cnt;
                numholder += cnt + 1;
                current_data_position = numholder + 1;
            }
        }
    
    	i = 0;
    	byte_value  = (byte)(maxsize & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)((maxsize >> 8) & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)(occupybits & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)((occupybits >> 8) & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)(data_size & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)((data_size >> 8) & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)(value_size & 0x000000FF);
    	dst[i++]    = byte_value;
    	byte_value  = (byte)((value_size >> 8) & 0x000000FF);
    	dst[i++]    = byte_value;
    	for(j = 0; j < maxsize + 1; j++)
    	{
    		byte_value  = (byte)(map[j] & 0x000000FF);
    		dst[i++]    = byte_value;
    		byte_value  = (byte)((map[j] >> 8) & 0x000000FF);
    		dst[i++]    = byte_value;
    	}
    	for(j = 0; j < data_size; j++)
    		dst[i++] = data[j];
    	for(j = 0; j < value_size; j++)
    		dst[i++] = value[j];
        NumberOfBytes = i;
    	return(NumberOfBytes);
    }

    public int PackWavelets(int src[], byte dst[], int xdim, int ydim, double amount_of_compression)
	{
		int i, number_of_nonzeros;
		forward_xform(src, xdim, ydim);
		threshold(src, xdim * ydim, amount_of_compression);
		int NumberOfBytes = pack(src, xdim, ydim, dst);
		return(NumberOfBytes);
	}
}
