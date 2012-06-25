elephant = DGSource("ElephantsDream-avc.dgx")
elephant = ConvertToYV12(elephant)
elephant = Crop(elephant, 0, 0, 640, 180)
elephant = Trim(elephant, 240, 539)
elephant = Subtitle(elephant, "width = " + String(width(elephant)) + " height = " + String(height(elephant)))

destination = DGSource("DestinationTruth.dgx")
destination = ConvertToYV12(destination)
destination = SWScale(destination, 640, 360, "Lanczos")
destination = Trim(destination, 0, 299)

dirtymary = DGSource("DirtyMary.dgx")
dirtymary = ConvertToYV12(dirtymary)
dirtymary = SWScale(dirtymary, 640, 360, "Lanczos")
dirtymary = Trim(dirtymary, 3000, 3299)

stackhorizontal(stackvertical(elephant, destination), stackvertical(dirtymary, elephant))
