class TrackTopTag():

    def __init__(self):

        self.name  = None 
        self.count = None
        self.url   = None
    
    def getName(self):

        return self.name

    def setName(self, name):

        self.name =name

    def getCount(self):

        return self.count

    def setCount(self, count):

        try:
            self.count = int(count)
        except:
            self.count = None 

    def getUrl(self):

        return self.url

    def setUrl(self, url):

        self.url = url

class SimilarTrackArtist():

    def __init__(self):

        self.name = None
        self.url  = None

    def getName(self):

        return self.name

    def setName(self, name):

        self.name = name

    def getUrl(self):

        return self.url
    
    def setUrl(self, url):

        self.url = url

class ArtistTopTag():

    def __init__(self):

        self.name  = None 
        self.count = None
        self.url   = None
    
    def getName(self):

        return self.name

    def setName(self, name):

        self.name =name

    def getCount(self):

        return self.count

    def setCount(self, count):

        try:
            self.count = int(count)
        except:
            self.count = None 

    def getUrl(self):

        return self.url

    def setUrl(self, url):

        self.url = url

class SimilarTrackArtist():

    def __init__(self):

        self.name = None
        self.url  = None

    def getName(self):

        return self.name

    def setName(self, name):

        self.name = name

    def getUrl(self):

        return self.url
    
    def setUrl(self, url):

        self.url = url

class SimilarTrack():

    def __init__(self):

        self.name  = None 
        self.match = None
        self.url   = None
        self.streamable = None
        self.artist = None

    def getArtist(self):

        return self.artist

    def setArtist(self, artist):

        self.artist = artist

    def getName(self):

        return self.name

    def setName(self, name):

        self.name = name

    def getMatch(self):

        return self.match

    def setMatch(self, match):

        try:
            self.match = float(match)
        except:
            self.match = None 

    def getUrl(self):

        return self.url

    def setUrl(self, url):

        self.url = url

    def getStreamable(self):

        return self.streamable

    def setStreamable(self, streamable):

        self.streamable = int(streamable)

class UserTopTrack():

    def __init__(self):

        self.artist         = None
        self.artist_mbid    = None
        self.name           = None 
        self.mbid           = None
        self.playcount      = None
        self.rank           = None
        self.url            = None

    def getArtist(self):
        return self.artist

    def setArtist(self, artist):
        self.artist = artist

    def getArtistMBID(self):
        return self.artist_mbid

    def setArtistMBID(self, artist_mbid):
        self.artist_mbid = artist_mbid

    def getName(self):
        return self.name

    def setName(self, name):
        self.name = name

    def getMBID(self):
        return self.mbid

    def setMBID(self, mbid):
        self.mbid = mbid

    def getPlaycount(self):
        return self.playcount

    def setPlaycount(self, pc):
        try:
            self.playcount = int(pc)
        except:
            self.playcount = None 

    def getUrl(self):

        return self.url

    def setUrl(self, url):

        self.url = url

    def getRank(self):

        return self.rank

    def setRank(self, rank):

        try:
            self.rank = int(rank)
        except:
            self.rank = 0

class SimilarArtist():

    def __init__(self):

        self.name           = None 
        self.mbid           = None
        self.match          = None
        self.url            = None
        self.image_small    = None
        self.image          = None
        self.streamable     = None

    def getName(self):
        return self.name

    def setName(self, name):
        self.name = name

    def getMBID(self):
        return self.mbid

    def setMBID(self, mbid):
        self.mbid = mbid

    def getMatch(self):
        return self.match

    def setMatch(self, match):
        try:
            self.match = float(match)
        except:
            self.match = None 

    def getUrl(self):
        return self.url

    def setUrl(self, url):
        self.url = url

    def getImageSmall(self):
        return self.image_small

    def setImageSmall(self, image_small):
        self.image_small = image_small

    def getImage(self):
        return self.image

    def setImage(self, image):
        self.image = image

    def getStreamable(self):
        return self.streamable

    def setStreamable(self, streamable):
        self.streamable = int(streamable)

class TagTopArtist():

    def __init__(self):

        self.name           = None 
        self.mbid           = None
        self.url            = None
        self.thumbnail      = None
        self.image          = None
        self.streamable     = None
        self.count          = None

    def getName(self):
        return self.name

    def setName(self, name):
        self.name = name

    def getMBID(self):
        return self.mbid

    def setMBID(self, mbid):
        self.mbid = mbid

    def getUrl(self):
        return self.url

    def setUrl(self, url):
        self.url = url

    def getThumbnail(self):
        return self.thumbnail

    def setThumbnail(self, thumbnail):
        self.image_small = thumbnail

    def getImage(self):
        return self.image

    def setImage(self, image):
        self.image = image

    def getStreamable(self):
        return self.streamable

    def setStreamable(self, streamable):
        self.streamable = int(streamable)

    def getCount(self):
        return self.count

    def setCount(self, count):
        self.count = int(count)

