library(mocap)
library(jsonlite)
library(parallel)

rawDir <- "./dataset/raw"
processedDir <- "./dataset/xyz"

process <- function(dir) {
    library(mocap)
    library(jsonlite)
    rawDir <- "./dataset/raw"
    processedDir <- "./dataset/xyz"
    dir.create(paste(processedDir, "/", dir, sep = "", collapse = NULL))
    asfFile <- list.files(path = paste(rawDir, "/", dir, sep = "", collapse = NULL), pattern = ".asf", full.names = T)
    print(asfFile)
    asf <- readASF(asfFile[1])

    amcFiles <- list.files(path = paste(rawDir, "/", dir, sep = "", collapse = NULL), pattern = ".amc", full.names = F)
    for (amcFile in amcFiles) {
        fullname <- paste(rawDir, "/", dir, "/", amcFile, sep = "", collapse = NULL)
        print(fullname)
        amc <- readAMC(fullname, asf)
        xyz <- getMotionData(asf, amc)
        json_str<-toJSON(xyz)
        cat(json_str, file=paste(processedDir, "/", dir, "/", substring(amcFile, 0, nchar(amcFile) - 4), ".json", sep = "", collapse = NULL))
    }
}

dir.create(processedDir)
dirs <- list.dirs(path = rawDir, full.names = F, recursive = F)

cl <- makeCluster(8)
results <- parLapply(cl, dirs, process)
final <- do.call('c',results)
stopCluster(cl)
print(final)