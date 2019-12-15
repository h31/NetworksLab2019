import java.io.File

import cats.effect.Sync

class Cache[F[_]: Sync] private () {
  private val store =
    scala.collection.mutable.Map[Int, File]()

  def findAll: F[List[(Int, File)]]      = Sync[F].delay(store.toList)
  def file(port: Int): F[Option[File]]   = Sync[F].delay(store.get(port))
  def delete(port: Int): F[Option[File]] = Sync[F].delay(store.remove(port))
  def add(tuple: (Int, File)): F[Unit]   = Sync[F].delay(store += tuple)
}

object Cache {
  def apply[F[_]: Sync](): F[Cache[F]] = Sync[F].delay(new Cache[F])
}
