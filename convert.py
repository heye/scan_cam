from PIL import Image



def main():

    image_height = 0 # will be determined by the datta
    image_width = 3000
    #image_width = 384

    #print file size
    file = open("values.bin", "rb")
    if not file:
        print("cannot open file")
    file_buf = file.read()
    print("file size: " + str(len(file_buf)))
    file.close()

    #partially read file into buffer
    image_buf = bytearray()
    with open('values.bin', 'rb') as file:
        pixel_data = file.read(2)
        pixel_data_trash = file.read(2)
        line_number = 0
        pixel_number = 0
        line_pixel_number = 0  


        dark_value = 11000
        light_value = 4500

        skip_pixels = 3825/image_width

        while pixel_data:
            #pixel = int.from_bytes(pixel_data, "little", signed=True)

            pixel_number += 1
            if pixel_number >= 3825:
                print("pixel in line " + str(line_number) + ": " + str(line_pixel_number))
                pixel_number = 0
                line_number += 1
                line_pixel_number = 0

            if pixel_number > skip_pixels*line_pixel_number:
                line_pixel_number += 1
                #print(pixel_data_h)
                pixel = int.from_bytes(pixel_data, byteorder='little')
                #print(pixel)
                if pixel < light_value:
                    pixel = light_value
                if pixel > dark_value:
                    pixel = dark_value
                pixel -= light_value
                pixel = (dark_value-light_value) - pixel

                #range/256
                pixel = int(pixel/((dark_value-light_value)/255))

                if line_pixel_number <= image_width:
                    image_buf.append(pixel)

            pixel_data = file.read(2)
            pixel_data_trash = file.read(2)
            
        image_height = line_number




    file.close()


    print("HEIGHT: " + str(image_height))
    print("WIDTH: " + str(image_width))


    #TODO: open binary file, read into buffer
    buf = bytearray()
    for y in range(0,256):
        for x in range(0,256):
            buf.append(y)

    #write buffer into file object
    image = Image.frombuffer('L', (image_width,image_height), image_buf)
    image = image.transpose(Image.FLIP_LEFT_RIGHT)
    image = image.transpose(Image.FLIP_TOP_BOTTOM)
    #image = Image.frombuffer('L', (256,256), buf)
    file = open("hello.bmp", "wb")
    image.save(file, 'BMP')

if __name__ == "__main__":
    main()