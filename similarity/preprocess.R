library(mocap)
library(jsonlite)

rawDir <- "./dataset/raw"
processedDir <- "./dataset/processed"

dir.create(processedDir)
dirs <- list.dirs(path = rawDir, full.names = F) 

d <- c(2:length(dirs))
for (i in d) {
    dir.create(paste(processedDir, "/", dirs[i], sep = "", collapse = NULL))
    asfFile <- list.files(path = paste(rawDir, "/", dirs[i], sep = "", collapse = NULL), pattern = ".asf", full.names = T)
    print(asfFile)
    asf <- readASF(asfFile[1])

    amcFiles <- list.files(path = paste(rawDir, "/", dirs[i], sep = "", collapse = NULL), pattern = ".amc", full.names = F)
    f <- c(1:length(amcFiles))
    for (j in f) {
        amcFile <- amcFiles[j]
        fullname <- paste(rawDir, "/", dirs[i], "/", amcFile, sep = "", collapse = NULL)
        print(fullname)
        amc <- readAMC(fullname, asf)
        xyz <- getMotionData(asf, amc)
        json_str<-toJSON(xyz)
        cat(json_str, file=paste(processedDir, "/", dirs[i], "/", j, ".json", sep = "", collapse = NULL))
    }
}