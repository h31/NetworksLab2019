name := "task3"

lazy val baseSettings = Seq(
    name := "task2"
  , organization := "ru.polytech.kspt"
  , version := "0.1.0-SNAPSHOT"
  , scalaVersion := "2.12.8"
  , sbtVersion := "1.2.8"
  , scalacOptions ++= Seq(
      "-feature"
    , "-unchecked"
    , "-deprecation"
    , "-Xfatal-warnings"
    , "-Ypartial-unification"
    , "-language:higherKinds"
  )
  , resourceDirectory in Compile := baseDirectory.value / "resources"
)

lazy val pureConfigVersion = "0.12.0"
lazy val scalaTestVersion  = "3.0.4"
lazy val logbackVersion    = "1.2.3"
lazy val catsVersion       = "2.0.0"

lazy val root = (project in file("."))
  .settings(
      baseSettings
    , libraryDependencies ++= Seq(
        "org.typelevel"         %% "cats-effect"    % catsVersion
      , "org.typelevel"         %% "cats-core"      % catsVersion
      , "org.scalatest"         %% "scalatest"      % scalaTestVersion
      , "ch.qos.logback"        % "logback-classic" % logbackVersion
      , "com.github.pureconfig" %% "pureconfig"     % pureConfigVersion
    ).map(_ withSources () withJavadoc ())
  )
